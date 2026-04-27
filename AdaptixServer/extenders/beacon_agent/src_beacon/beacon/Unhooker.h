#pragma once

/*
 * Unhooker.h - EDR Hook Removal
 *
 * Provides functionality to remove EDR hooks by restoring clean
 * DLL code from KnownDlls or from disk.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Unhooking method types
typedef enum _UNHOOK_METHOD {
    UNHOOK_METHOD_KNOWNDLLS = 0,    // Use \KnownDlls\ntdll.dll
    UNHOOK_METHOD_DISK = 1,          // Read from system32 on disk
    UNHOOK_METHOD_MEMORY = 2,        // Use fresh copy from another process
    UNHOOK_METHOD_SYSCALL = 3        // Use syscall to unmap/remap
} UNHOOK_METHOD;

// Unhook status
typedef struct _UNHOOK_STATE {
    BOOL    bNtdllUnhooked;
    BOOL    bKernel32Unhooked;
    PVOID   pCleanNtdll;
    SIZE_T  ntdllSize;
} UNHOOK_STATE, *PUNHOOK_STATE;

// Global unhook state
extern UNHOOK_STATE g_UnhookState;

// Function declarations

/**
 * Unhook ntdll.dll by restoring from KnownDlls
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UnhookNtdll();

/**
 * Unhook ntdll.dll using specified method
 * @param method Unhooking method to use
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UnhookNtdllEx(UNHOOK_METHOD method);

/**
 * Unhook kernel32.dll by restoring from disk
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UnhookKernel32();

/**
 * Unhook all common EDR-targeted DLLs
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UnhookAll();

/**
 * Unhook specific DLL
 * @param dllName Name of DLL to unhook
 * @return TRUE if successful, FALSE otherwise
 */
BOOL UnhookDll(LPCSTR dllName);

/**
 * Check if ntdll is currently hooked
 * @return TRUE if hooked, FALSE if clean
 */
BOOL IsNtdllHooked();

/**
 * Check if specific function is hooked
 * @param funcAddress Address of function to check
 * @return TRUE if hooked, FALSE if clean
 */
BOOL IsFunctionHooked(PVOID funcAddress);

/**
 * Get clean ntdll from KnownDlls
 * @param ppBuffer Output buffer for clean ntdll
 * @param pSize Output size
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GetCleanNtdllFromKnownDlls(PVOID* ppBuffer, PSIZE_T pSize);

/**
 * Get clean DLL from disk
 * @param dllPath Path to DLL on disk
 * @param ppBuffer Output buffer
 * @param pSize Output size
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GetCleanDllFromDisk(LPCWSTR dllPath, PVOID* ppBuffer, PSIZE_T pSize);

/**
 * Restore function from clean DLL
 * @param pTargetFunc Function in memory to restore
 * @param pCleanDll Clean DLL buffer
 * @param funcHash DJB2 hash of function name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RestoreFunction(PVOID pTargetFunc, PVOID pCleanDll, ULONG funcHash);

/**
 * Restore all hooked functions in a DLL
 * @param pTargetDll Target DLL base address
 * @param pCleanDll Clean DLL buffer
 * @return Number of functions restored, -1 on error
 */
INT RestoreAllFunctions(PVOID pTargetDll, PVOID pCleanDll);

/**
 * Clean unhook state and free resources
 */
void CleanupUnhookState();