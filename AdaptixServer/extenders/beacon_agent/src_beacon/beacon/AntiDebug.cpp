#include "AntiDebug.h"
#include "utils.h"
#include "ntdll.h"
#include <intrin.h>

// Check PEB.BeingDebugged
BOOL CheckDebuggerPEB() {
#ifdef _M_IX86
    PEB* peb = (PEB*)__readfsdword(0x30);
#else
    PEB* peb = (PEB*)__readgsqword(0x60);
#endif

    return peb->BeingDebugged != 0;
}

// Check PEB.NtGlobalFlag
BOOL CheckDebuggerNtGlobalFlag() {
#ifdef _M_IX86
    PEB* peb = (PEB*)__readfsdword(0x30);
#else
    PEB* peb = (PEB*)__readgsqword(0x60);
#endif

    // FLG_HEAP_ENABLE_TAIL_CHECK (0x10)
    // FLG_HEAP_ENABLE_FREE_CHECK (0x20)
    // FLG_HEAP_VALIDATE_PARAMETERS (0x40)
    // These are set when process is debugged

    const ULONG debuggerFlags = 0x70;
    return (peb->NtGlobalFlag & debuggerFlags) != 0;
}

// Check heap flags
BOOL CheckDebuggerHeapFlags() {
#ifdef _M_IX86
    PEB* peb = (PEB*)__readfsdword(0x30);
#else
    PEB* peb = (PEB*)__readgsqword(0x60);
#endif

    // Check ProcessHeap flags
    PVOID heap = peb->ProcessHeap;
    if (!heap) return FALSE;

    // HEAP flags at offset 0x40 (x64) or 0x0C (x32)
#ifdef _M_IX86
    DWORD flags = *(DWORD*)((BYTE*)heap + 0x0C);
    DWORD forceFlags = *(DWORD*)((BYTE*)heap + 0x10);
#else
    DWORD flags = *(DWORD*)((BYTE*)heap + 0x70);
    DWORD forceFlags = *(DWORD*)((BYTE*)heap + 0x74);
#endif

    // HEAP_GROWABLE (0x2) - normal
    // HEAP_TAIL_CHECKING_ENABLED (0x20) - debugger
    // HEAP_FREE_CHECKING_ENABLED (0x40) - debugger
    // HEAP_VALIDATE_PARAMETERS_ENABLED (0x40000000) - debugger

    const DWORD debugHeapFlags = 0x40000060;
    return (flags & debugHeapFlags) != 0 || (forceFlags & debugHeapFlags) != 0;
}

// Check remote debugger using CheckRemoteDebuggerPresent
BOOL CheckRemoteDebugger() {
    if (!SysModules || !SysModules->Kernel32) return FALSE;

    typedef BOOL(WINAPI* CheckRemoteDebuggerPresent_t)(HANDLE, PBOOL);
    CheckRemoteDebuggerPresent_t pCheckRemoteDebuggerPresent =
        (CheckRemoteDebuggerPresent_t)GetProcAddress(SysModules->Kernel32, "CheckRemoteDebuggerPresent");

    if (pCheckRemoteDebuggerPresent) {
        BOOL bDebuggerPresent = FALSE;
        if (pCheckRemoteDebuggerPresent((HANDLE)-1, &bDebuggerPresent)) {
            return bDebuggerPresent;
        }
    }

    return FALSE;
}

// Check hardware breakpoints using debug registers
BOOL CheckHardwareBreakpoints() {
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    // Get thread context
    HANDLE hThread = (HANDLE)-1;
    typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE, PCONTEXT);
    NtGetContextThread_t pNtGetContextThread =
        (NtGetContextThread_t)GetProcAddress(SysModules->Ntdll, "NtGetContextThread");

    if (pNtGetContextThread) {
        NTSTATUS status = pNtGetContextThread(hThread, &ctx);
        if (NT_SUCCESS(status)) {
            // Check DR0-DR3 for breakpoints
            return ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0;
        }
    }

    // Alternative using __getDR
#ifdef _M_X64
    // Can't directly read DR registers in user mode
    // Use GetThreadContext
#endif

    return FALSE;
}

// Get hardware breakpoint status
void GetHardwareBreakpointStatus(PVOID* pDr0, PVOID* pDr1, PVOID* pDr2, PVOID* pDr3, DWORD* pDr6, DWORD* pDr7) {
    if (pDr0) *pDr0 = NULL;
    if (pDr1) *pDr1 = NULL;
    if (pDr2) *pDr2 = NULL;
    if (pDr3) *pDr3 = NULL;
    if (pDr6) *pDr6 = 0;
    if (pDr7) *pDr7 = 0;

    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE, PCONTEXT);
    NtGetContextThread_t pNtGetContextThread =
        (NtGetContextThread_t)GetProcAddress(SysModules->Ntdll, "NtGetContextThread");

    if (pNtGetContextThread) {
        NTSTATUS status = pNtGetContextThread((HANDLE)-1, &ctx);
        if (NT_SUCCESS(status)) {
            if (pDr0) *pDr0 = (PVOID)ctx.Dr0;
            if (pDr1) *pDr1 = (PVOID)ctx.Dr1;
            if (pDr2) *pDr2 = (PVOID)ctx.Dr2;
            if (pDr3) *pDr3 = (PVOID)ctx.Dr3;
            if (pDr6) *pDr6 = ctx.Dr6;
            if (pDr7) *pDr7 = ctx.Dr7;
        }
    }
}

// Clear hardware breakpoints
BOOL ClearHardwareBreakpoints() {
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    typedef NTSTATUS(NTAPI* NtGetContextThread_t)(HANDLE, PCONTEXT);
    typedef NTSTATUS(NTAPI* NtSetContextThread_t)(HANDLE, PCONTEXT);

    NtGetContextThread_t pNtGetContextThread =
        (NtGetContextThread_t)GetProcAddress(SysModules->Ntdll, "NtGetContextThread");
    NtSetContextThread_t pNtSetContextThread =
        (NtSetContextThread_t)GetProcAddress(SysModules->Ntdll, "NtSetContextThread");

    if (!pNtGetContextThread || !pNtSetContextThread) return FALSE;

    NTSTATUS status = pNtGetContextThread((HANDLE)-1, &ctx);
    if (!NT_SUCCESS(status)) return FALSE;

    // Clear all debug registers
    ctx.Dr0 = 0;
    ctx.Dr1 = 0;
    ctx.Dr2 = 0;
    ctx.Dr3 = 0;
    ctx.Dr6 = 0;
    ctx.Dr7 = 0;

    status = pNtSetContextThread((HANDLE)-1, &ctx);
    return NT_SUCCESS(status);
}

// Check software breakpoints (INT3 = 0xCC)
BOOL CheckSoftwareBreakpoints(PVOID pCode, SIZE_T size) {
    if (!pCode || size == 0) return FALSE;

    BYTE* pBytes = (BYTE*)pCode;
    for (SIZE_T i = 0; i < size; i++) {
        if (pBytes[i] == 0xCC) {
            return TRUE;
        }
    }

    return FALSE;
}

// Check if parent process is explorer.exe
BOOL CheckParentProcess() {
    // Get parent process ID from PEB
    typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NtQueryInformationProcess_t pNtQueryInformationProcess =
        (NtQueryInformationProcess_t)GetProcAddress(SysModules->Ntdll, "NtQueryInformationProcess");

    if (!pNtQueryInformationProcess) {
        // Assume normal if can't check
        return FALSE;
    }

    typedef struct _PROCESS_BASIC_INFORMATION {
        NTSTATUS ExitStatus;
        PVOID PebBaseAddress;
        ULONG_PTR AffinityMask;
        LONG_PTR BasePriority;
        ULONG_PTR UniqueProcessId;
        ULONG_PTR InheritedFromUniqueProcessId;
    } PROCESS_BASIC_INFORMATION;

    PROCESS_BASIC_INFORMATION pbi = { 0 };
    NTSTATUS status = pNtQueryInformationProcess((HANDLE)-1, 0, &pbi, sizeof(pbi), NULL);

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    DWORD parentId = (DWORD)pbi.InheritedFromUniqueProcessId;

    // Check if parent is explorer.exe
    // Create toolhelp snapshot
    typedef HANDLE(WINAPI* CreateToolhelp32Snapshot_t)(DWORD, DWORD);
    typedef BOOL(WINAPI* Process32FirstW_t)(HANDLE, LPPROCESSENTRY32W);
    typedef BOOL(WINAPI* Process32NextW_t)(HANDLE, LPPROCESSENTRY32W);

    CreateToolhelp32Snapshot_t pCreateToolhelp32Snapshot =
        (CreateToolhelp32Snapshot_t)GetProcAddress(SysModules->Kernel32, "CreateToolhelp32Snapshot");
    Process32FirstW_t pProcess32FirstW =
        (Process32FirstW_t)GetProcAddress(SysModules->Kernel32, "Process32FirstW");
    Process32NextW_t pProcess32NextW =
        (Process32NextW_t)GetProcAddress(SysModules->Kernel32, "Process32NextW");

    if (!pCreateToolhelp32Snapshot || !pProcess32FirstW || !pProcess32NextW) {
        return FALSE;
    }

    HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return FALSE;

    PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
    BOOL found = FALSE;
    BOOL isExplorer = FALSE;

    if (pProcess32FirstW(hSnapshot, &pe)) {
        do {
            if (pe.th32ProcessID == parentId) {
                found = TRUE;
                // Check if name is explorer.exe
                WCHAR explorerName[] = { L'e', L'x', L'p', L'l', L'o', L'r', L'e', L'r', L'.', L'e', L'x', L'e', 0 };
                isExplorer = (StrCmpLowW(pe.szExeFile, explorerName) == 0);
                break;
            }
        } while (pProcess32NextW(hSnapshot, &pe));
    }

    ApiNt->NtClose(hSnapshot);

    // Return TRUE if parent is NOT explorer (suspicious)
    if (found && !isExplorer) {
        return TRUE;
    }

    return FALSE;
}

// Timing check
BOOL CheckTiming(DWORD thresholdMs) {
    DWORD start = ApiWin->GetTickCount();

    // Do some work
    for (volatile int i = 0; i < 1000; i++) {
        // Empty loop
    }

    DWORD elapsed = ApiWin->GetTickCount() - start;

    // If elapsed time is much larger than expected, debugger might be slowing us
    return elapsed > thresholdMs;
}

// Check debug port using NtQueryInformationProcess
BOOL CheckDebugPort() {
    if (!SysModules || !SysModules->Ntdll) return FALSE;

    typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NtQueryInformationProcess_t pNtQueryInformationProcess =
        (NtQueryInformationProcess_t)GetProcAddress(SysModules->Ntdll, "NtQueryInformationProcess");

    if (!pNtQueryInformationProcess) return FALSE;

    HANDLE debugPort = NULL;
    NTSTATUS status = pNtQueryInformationProcess((HANDLE)-1, 7, // ProcessDebugPort
        &debugPort, sizeof(debugPort), NULL);

    if (NT_SUCCESS(status) && debugPort != NULL) {
        return TRUE;
    }

    return FALSE;
}

// Check using NtQueryInformationProcess with various info classes
BOOL CheckNtQueryInfoProcess() {
    if (!SysModules || !SysModules->Ntdll) return FALSE;

    typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NtQueryInformationProcess_t pNtQueryInformationProcess =
        (NtQueryInformationProcess_t)GetProcAddress(SysModules->Ntdll, "NtQueryInformationProcess");

    if (!pNtQueryInformationProcess) return FALSE;

    // ProcessDebugObjectHandle (30)
    HANDLE debugObject = NULL;
    NTSTATUS status = pNtQueryInformationProcess((HANDLE)-1, 30,
        &debugObject, sizeof(debugObject), NULL);

    // Success means debugger is present
    if (NT_SUCCESS(status)) {
        return TRUE;
    }

    // ProcessDebugFlags (31)
    ULONG debugFlags = 0;
    status = pNtQueryInformationProcess((HANDLE)-1, 31,
        &debugFlags, sizeof(debugFlags), NULL);

    // If flags == 0, process is being debugged
    if (NT_SUCCESS(status) && debugFlags == 0) {
        return TRUE;
    }

    return FALSE;
}

// Simple debugger check
BOOL IsDebuggerPresentCustom() {
    // Use Windows API first
    typedef BOOL(WINAPI* IsDebuggerPresent_t)();
    IsDebuggerPresent_t pIsDebuggerPresent =
        (IsDebuggerPresent_t)GetProcAddress(SysModules->Kernel32, "IsDebuggerPresent");

    if (pIsDebuggerPresent && pIsDebuggerPresent()) {
        return TRUE;
    }

    // Check PEB directly
    return CheckDebuggerPEB();
}

// Check for kernel debugger
BOOL CheckKernelDebugger() {
    if (!SysModules || !SysModules->Ntdll) return FALSE;

    typedef NTSTATUS(NTAPI* NtQuerySystemInformation_t)(ULONG, PVOID, ULONG, PULONG);
    NtQuerySystemInformation_t pNtQuerySystemInformation =
        (NtQuerySystemInformation_t)GetProcAddress(SysModules->Ntdll, "NtQuerySystemInformation");

    if (!pNtQuerySystemInformation) return FALSE;

    typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
        BOOLEAN KernelDebuggerEnabled;
        BOOLEAN KernelDebuggerNotPresent;
    } SYSTEM_KERNEL_DEBUGGER_INFORMATION;

    SYSTEM_KERNEL_DEBUGGER_INFORMATION kdi = { 0 };
    NTSTATUS status = pNtQuerySystemInformation(35, // SystemKernelDebuggerInformation
        &kdi, sizeof(kdi), NULL);

    if (NT_SUCCESS(status)) {
        return kdi.KernelDebuggerEnabled && !kdi.KernelDebuggerNotPresent;
    }

    return FALSE;
}

// Block debugger using NtSetInformationThread
BOOL BlockDebugger() {
    if (!SysModules || !SysModules->Ntdll) return FALSE;

    typedef NTSTATUS(NTAPI* NtSetInformationThread_t)(HANDLE, ULONG, PVOID, ULONG);
    NtSetInformationThread_t pNtSetInformationThread =
        (NtSetInformationThread_t)GetProcAddress(SysModules->Ntdll, "NtSetInformationThread");

    if (!pNtSetInformationThread) return FALSE;

    // ThreadHideFromDebugger = 0x11
    NTSTATUS status = pNtSetInformationThread((HANDLE)-2, // Current thread
        0x11, NULL, 0);

    return NT_SUCCESS(status);
}

// Self-debug check
BOOL SelfDebugCheck() {
    if (!SysModules || !SysModules->Kernel32) return FALSE;

    typedef BOOL(WINAPI* DebugActiveProcess_t)(DWORD);
    typedef BOOL(WINAPI* DebugActiveProcessStop_t)(DWORD);

    DebugActiveProcess_t pDebugActiveProcess =
        (DebugActiveProcess_t)GetProcAddress(SysModules->Kernel32, "DebugActiveProcess");
    DebugActiveProcessStop_t pDebugActiveProcessStop =
        (DebugActiveProcessStop_t)GetProcAddress(SysModules->Kernel32, "DebugActiveProcessStop");

    if (!pDebugActiveProcess) return FALSE;

    // Try to debug ourselves - will fail if already being debugged
    DWORD pid = (DWORD)NtCurrentTeb()->ClientId.UniqueProcess;
    BOOL result = pDebugActiveProcess(pid);

    if (result) {
        // Not being debugged, stop debugging ourselves
        if (pDebugActiveProcessStop) {
            pDebugActiveProcessStop(pid);
        }
        return FALSE;
    }

    // Failed to debug ourselves - we're being debugged
    return TRUE;
}

// Check SeDebugPrivilege
BOOL CheckSeDebugPrivilege() {
    HANDLE hToken = NULL;

    if (!NT_SUCCESS(ApiNt->NtOpenProcessToken(NtCurrentProcess(), TOKEN_QUERY, &hToken))) {
        return FALSE;
    }

    // Query for SeDebugPrivilege
    LUID luid = { 0 };
    typedef BOOL(WINAPI* LookupPrivilegeValueA_t)(LPCSTR, LPCSTR, PLUID);
    LookupPrivilegeValueA_t pLookupPrivilegeValueA =
        (LookupPrivilegeValueA_t)GetProcAddress(SysModules->Advapi32, "LookupPrivilegeValueA");

    if (!pLookupPrivilegeValueA) {
        ApiNt->NtClose(hToken);
        return FALSE;
    }

    char debugPriv[] = { 'S', 'e', 'D', 'e', 'b', 'u', 'g', 'P', 'r', 'i', 'v', 'i', 'l', 'e', 'g', 'e', 0 };
    pLookupPrivilegeValueA(NULL, debugPriv, &luid);

    PRIVILEGE_SET ps = { 0 };
    ps.PrivilegeCount = 1;
    ps.Control = PRIVILEGE_SET_ALL_NECESSARY;
    ps.Privilege[0].Luid = luid;
    ps.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bResult = FALSE;
    typedef BOOL(WINAPI* AccessCheckAndAuditAlarmA_t)(LPCSTR, LPVOID, LPSTR, LPSTR, PSECURITY_DESCRIPTOR,
        DWORD, PGENERIC_MAPPING, BOOL, DWORD, DWORD, BOOL);
    // Alternative: PrivilegeCheck

    typedef BOOL(WINAPI* PrivilegeCheck_t)(HANDLE, PPRIVILEGE_SET, LPBOOL);
    PrivilegeCheck_t pPrivilegeCheck =
        (PrivilegeCheck_t)GetProcAddress(SysModules->Advapi32, "PrivilegeCheck");

    if (pPrivilegeCheck) {
        pPrivilegeCheck(hToken, &ps, &bResult);
    }

    ApiNt->NtClose(hToken);
    return bResult;
}

// Comprehensive debugger check
BOOL CheckDebugger(DWORD checks, PDEBUG_CHECK_RESULT pResult) {
    if (!pResult) return FALSE;

    memset(pResult, 0, sizeof(DEBUG_CHECK_RESULT));

    BOOL detected = FALSE;

    if (checks & DEBUG_CHECK_PEB_BEINGDEBUGGED) {
        if (CheckDebuggerPEB()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_PEB_BEINGDEBUGGED;
        }
    }

    if (checks & DEBUG_CHECK_PEB_NtGLOBALFLAG) {
        if (CheckDebuggerNtGlobalFlag()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_PEB_NtGLOBALFLAG;
        }
    }

    if (checks & DEBUG_CHECK_HEAP_FLAGS) {
        if (CheckDebuggerHeapFlags()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_HEAP_FLAGS;
        }
    }

    if (checks & DEBUG_CHECK_REMOTE_DEBUGGER) {
        if (CheckRemoteDebugger()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_REMOTE_DEBUGGER;
        }
    }

    if (checks & DEBUG_CHECK_HARDWARE_BPS) {
        if (CheckHardwareBreakpoints()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_HARDWARE_BPS;
        }
    }

    if (checks & DEBUG_CHECK_PARENT_PROCESS) {
        if (CheckParentProcess()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_PARENT_PROCESS;
        }
    }

    if (checks & DEBUG_CHECK_DEBUG_PORT) {
        if (CheckDebugPort()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_DEBUG_PORT;
        }
    }

    if (checks & DEBUG_CHECK_NT_QUERY_INFO) {
        if (CheckNtQueryInfoProcess()) {
            detected = TRUE;
            pResult->detectedMethods |= DEBUG_CHECK_NT_QUERY_INFO;
        }
    }

    pResult->bDebuggerDetected = detected;

    return detected;
}

// TLS callbacks check (placeholder)
BOOL CheckTLSCallbacks() {
    // TLS callbacks can be used to detect debuggers at startup
    // This is a placeholder for actual TLS callback implementation
    return FALSE;
}