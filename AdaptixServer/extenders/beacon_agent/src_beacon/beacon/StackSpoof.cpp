#include "StackSpoof.h"
#include "utils.h"
#include "ntdll.h"
#include <intrin.h>

// Global spoofed call function pointer
SpoofedCall_t g_pSpoofedCall = NULL;

// Pre-computed legitimate return addresses
static LEGITIMATE_ADDRESS g_LegitAddresses[10] = { 0 };
static INT g_NumLegitAddresses = 0;
static BOOL g_Initialized = FALSE;

// Assembly for stack spoofing (x64)
// This trampoline replaces the return address on stack
extern "C" PVOID SpoofCallStub(PVOID targetFunc, PVOID spoofReturn, PVOID stackFrame, PVOID* args) {
    // x64 calling convention: rcx, rdx, r8, r9 then stack
    // We need to:
    // 1. Save current return address
    // 2. Replace with spoofed return address
    // 3. Call target function
    // 4. Restore return address

    PVOID result = NULL;

    __try {
        // Get current stack pointer
        PVOID* pStack = (PVOID*)_AddressOfReturnAddress();
        PVOID originalReturn = *(pStack - 1);

        // Replace return address with spoofed one
        *(pStack - 1) = spoofReturn;

        // Call the target function
        // Note: This is simplified - real implementation needs assembly
        typedef PVOID(*FuncPtr_t)();
        FuncPtr_t pFunc = (FuncPtr_t)targetFunc;
        result = pFunc();

        // Restore original return address
        *(pStack - 1) = originalReturn;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // Handle exception
    }

    return result;
}

// Initialize stack spoofing
BOOL InitStackSpoof() {
    if (g_Initialized) {
        return TRUE;
    }

    // Find legitimate return addresses in system DLLs
    HMODULE hKernel32 = SysModules ? SysModules->Kernel32 : NULL;
    HMODULE hNtdll = SysModules ? SysModules->Ntdll : NULL;

    if (!hKernel32) hKernel32 = GetModuleAddress(HASH_LIB_KERNEL32);
    if (!hNtdll) hNtdll = GetModuleAddress(HASH_LIB_NTDLL);

    // Get module sizes
    if (hKernel32) {
        PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hKernel32 +
            ((PIMAGE_DOS_HEADER)hKernel32)->e_lfanew);
        DWORD imageSize = pNtHeaders->OptionalHeader.SizeOfImage;

        // Find a suitable function in kernel32 for return address
        PVOID pSleep = GetSymbolAddress(hKernel32, HASH_FUNC_SLEEP);
        PVOID pGetTickCount = GetSymbolAddress(hKernel32, HASH_FUNC_GETTICKCOUNT);
        PVOID pVirtualAlloc = GetSymbolAddress(hKernel32, HASH_FUNC_VIRTUALALLOC);

        if (pSleep && g_NumLegitAddresses < 10) {
            // Use address after prologue (skip first few bytes)
            g_LegitAddresses[g_NumLegitAddresses].address = (PVOID)((BYTE*)pSleep + 5);
            memcpy(g_LegitAddresses[g_NumLegitAddresses].moduleName, "kernel32.dll", 12);
            memcpy(g_LegitAddresses[g_NumLegitAddresses].functionName, "Sleep", 5);
            g_NumLegitAddresses++;
        }

        if (pGetTickCount && g_NumLegitAddresses < 10) {
            g_LegitAddresses[g_NumLegitAddresses].address = (PVOID)((BYTE*)pGetTickCount + 3);
            memcpy(g_LegitAddresses[g_NumLegitAddresses].moduleName, "kernel32.dll", 12);
            memcpy(g_LegitAddresses[g_NumLegitAddresses].functionName, "GetTickCount", 12);
            g_NumLegitAddresses++;
        }
    }

    if (hNtdll && g_NumLegitAddresses < 10) {
        PVOID pNtClose = GetSymbolAddress(hNtdll, HASH_FUNC_NTCLOSE);
        if (pNtClose) {
            g_LegitAddresses[g_NumLegitAddresses].address = (PVOID)((BYTE*)pNtClose + 5);
            memcpy(g_LegitAddresses[g_NumLegitAddresses].moduleName, "ntdll.dll", 8);
            memcpy(g_LegitAddresses[g_NumLegitAddresses].functionName, "NtClose", 7);
            g_NumLegitAddresses++;
        }
    }

    g_Initialized = TRUE;
    return TRUE;
}

// Get legitimate return address from module
PVOID GetLegitimateReturnAddress(LPCSTR moduleName) {
    if (!g_Initialized) InitStackSpoof();

    for (INT i = 0; i < g_NumLegitAddresses; i++) {
        if (StrCmpLowA((CHAR*)moduleName, g_LegitAddresses[i].moduleName) == 0) {
            return g_LegitAddresses[i].address;
        }
    }

    return NULL;
}

// Get kernel32 return address
PVOID GetKernel32ReturnAddress() {
    return GetLegitimateReturnAddress("kernel32.dll");
}

// Get ntdll return address
PVOID GetNtdllReturnAddress() {
    return GetLegitimateReturnAddress("ntdll.dll");
}

// Get random system return address
PVOID GetRandomSystemReturnAddress() {
    if (!g_Initialized) InitStackSpoof();

    if (g_NumLegitAddresses == 0) {
        return NULL;
    }

    ULONG index = GenerateRandom32() % g_NumLegitAddresses;
    return g_LegitAddresses[index].address;
}

// Main spoof call implementation
PVOID SpoofCall(PVOID targetFunc, PVOID spoofReturn, ...) {
    if (!targetFunc) return NULL;
    if (!g_Initialized) InitStackSpoof();

    // Get variadic arguments
    va_list args;
    va_start(args, spoofReturn);

    // For x64, first 4 args go in registers (rcx, rdx, r8, r9)
    // We need assembly to properly handle this

    PVOID result = NULL;

    // Simplified implementation using exception handling
    __try {
        // Get stack frame pointer
        PVOID* pFrame = (PVOID*)_AddressOfReturnAddress();

        // Save original return address
        PVOID savedReturn = *(pFrame - 1);

        // Replace with spoofed return
        *(pFrame - 1) = spoofReturn;

        // Call target (this needs proper assembly for x64)
        // For now, call with no arguments
        typedef PVOID(*Func_t)();
        Func_t pFunc = (Func_t)targetFunc;
        result = pFunc();

        // Restore return address
        *(pFrame - 1) = savedReturn;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        result = NULL;
    }

    va_end(args);

    return result;
}

// Execute with spoofed configuration
PVOID ExecuteSpoofed(PSPOOF_CONFIG pConfig) {
    if (!pConfig || !pConfig->pTargetFunction) {
        return NULL;
    }

    PVOID returnAddr = pConfig->pSpoofReturn;
    if (!returnAddr) {
        returnAddr = GetRandomSystemReturnAddress();
    }

    return SpoofCall(pConfig->pTargetFunction, returnAddr);
}

// Backup stack state
BOOL BackupStack(PSPOOF_CONFIG pConfig) {
    if (!pConfig) return FALSE;

    // Allocate backup buffer
    pConfig->stackSize = 0x1000; // 4KB should be enough
    pConfig->pStackBackup = MemAllocLocal((DWORD)pConfig->stackSize);

    if (!pConfig->pStackBackup) {
        return FALSE;
    }

    // Copy current stack
    PVOID* pFrame = (PVOID*)_AddressOfReturnAddress();
    memcpy(pConfig->pStackBackup, (PVOID)(pFrame - 0x100), pConfig->stackSize);

    return TRUE;
}

// Restore stack state
BOOL RestoreStack(PSPOOF_CONFIG pConfig) {
    if (!pConfig || !pConfig->pStackBackup) {
        return FALSE;
    }

    // Restore stack
    PVOID* pFrame = (PVOID*)_AddressOfReturnAddress();
    memcpy((PVOID)(pFrame - 0x100), pConfig->pStackBackup, pConfig->stackSize);

    // Free backup
    memset(pConfig->pStackBackup, 0, pConfig->stackSize);
    MemFreeLocal(&pConfig->pStackBackup, (DWORD)pConfig->stackSize);
    pConfig->pStackBackup = NULL;

    return TRUE;
}

// Find suitable return address in module
PVOID FindSuitableReturnAddress(HMODULE hModule, DWORD minOffset, DWORD maxOffset) {
    if (!hModule) return NULL;

    // Get module size
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule +
        ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
    DWORD imageSize = pNtHeaders->OptionalHeader.SizeOfImage;

    if (minOffset >= imageSize || maxOffset > imageSize) {
        return NULL;
    }

    // Find a valid code location
    BYTE* pBase = (BYTE*)hModule;
    for (DWORD offset = minOffset; offset < maxOffset; offset++) {
        // Look for function prologue patterns
        BYTE b1 = pBase[offset];
        BYTE b2 = pBase[offset + 1];

        // Common x64 prologue patterns:
        // 48 89 5C 24 - mov [rsp+XX], rbx
        // 48 83 EC XX - sub rsp, XX
        // 40 53 - push rbx (with REX prefix)
        // 55 - push rbp

        if ((b1 == 0x48 && (b2 == 0x89 || b2 == 0x83)) ||
            (b1 == 0x40 && b2 == 0x53) ||
            (b1 == 0x55)) {
            // Found potential function start, return address after prologue
            return (PVOID)(pBase + offset + 5);
        }
    }

    return NULL;
}