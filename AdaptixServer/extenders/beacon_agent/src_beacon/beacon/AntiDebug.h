#pragma once

/*
 * AntiDebug.h - Anti-Debugging Techniques
 *
 * Provides multiple techniques to detect and evade debuggers
 * and analysis tools.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Debug detection method flags
typedef enum _DEBUG_CHECK {
    DEBUG_CHECK_PEB_BEINGDEBUGGED   = 0x0001,
    DEBUG_CHECK_PEB_NtGLOBALFLAG    = 0x0002,
    DEBUG_CHECK_HEAP_FLAGS          = 0x0004,
    DEBUG_CHECK_REMOTE_DEBUGGER     = 0x0008,
    DEBUG_CHECK_HARDWARE_BPS        = 0x0010,
    DEBUG_CHECK_SOFTWARE_BPS        = 0x0020,
    DEBUG_CHECK_PARENT_PROCESS      = 0x0040,
    DEBUG_CHECK_TIMING              = 0x0080,
    DEBUG_CHECK_DEBUG_PORT          = 0x0100,
    DEBUG_CHECK_NT_QUERY_INFO       = 0x0200,
    DEBUG_CHECK_ALL                 = 0xFFFF
} DEBUG_CHECK;

// Debug detection result
typedef struct _DEBUG_CHECK_RESULT {
    BOOL bDebuggerDetected;
    DWORD detectedMethods;         // Bitmask of DEBUG_CHECK flags
    CHAR detectedNames[256];       // Human-readable names
} DEBUG_CHECK_RESULT, *PDEBUG_CHECK_RESULT;

// Function declarations

/**
 * Check for debugger using PEB.BeingDebugged
 * @return TRUE if debugger detected, FALSE otherwise
 */
BOOL CheckDebuggerPEB();

/**
 * Check for debugger using PEB.NtGlobalFlag
 * @return TRUE if debugger detected, FALSE otherwise
 */
BOOL CheckDebuggerNtGlobalFlag();

/**
 * Check for debugger using heap flags
 * @return TRUE if debugger detected, FALSE otherwise
 */
BOOL CheckDebuggerHeapFlags();

/**
 * Check for remote debugger
 * @return TRUE if remote debugger detected, FALSE otherwise
 */
BOOL CheckRemoteDebugger();

/**
 * Check for hardware breakpoints
 * @return TRUE if hardware breakpoints detected, FALSE otherwise
 */
BOOL CheckHardwareBreakpoints();

/**
 * Check for software breakpoints (INT3)
 * @param pCode Code to check
 * @param size Size of code
 * @return TRUE if software breakpoints detected, FALSE otherwise
 */
BOOL CheckSoftwareBreakpoints(PVOID pCode, SIZE_T size);

/**
 * Check if parent process is explorer.exe
 * @return TRUE if parent is NOT explorer (suspicious), FALSE if it is explorer
 */
BOOL CheckParentProcess();

/**
 * Check for timing anomalies
 * @param thresholdMs Threshold in milliseconds
 * @return TRUE if timing anomaly detected (debugger), FALSE otherwise
 */
BOOL CheckTiming(DWORD thresholdMs);

/**
 * Check for debug port using NtQueryInformationProcess
 * @return TRUE if debug port found, FALSE otherwise
 */
BOOL CheckDebugPort();

/**
 * Check using NtQueryInformationProcess
 * @return TRUE if debugger detected, FALSE otherwise
 */
BOOL CheckNtQueryInfoProcess();

/**
 * Perform comprehensive debugger check
 * @param checks Bitmask of checks to perform
 * @param pResult Output result structure
 * @return TRUE if any debugger detected, FALSE otherwise
 */
BOOL CheckDebugger(DWORD checks, PDEBUG_CHECK_RESULT pResult);

/**
 * Simple debugger check (returns BOOL only)
 * @return TRUE if debugger detected, FALSE otherwise
 */
BOOL IsDebuggerPresentCustom();

/**
 * Check for kernel debugger
 * @return TRUE if kernel debugger detected, FALSE otherwise
 */
BOOL CheckKernelDebugger();

/**
 * Attempt to block debugger attachment
 * @return TRUE if successful, FALSE otherwise
 */
BOOL BlockDebugger();

/**
 * Self-debug attempt (detects debugger)
 * @return TRUE if debugger detected, FALSE otherwise
 */
BOOL SelfDebugCheck();

/**
 * Check for TLS callbacks (debugger detection)
 * @return TRUE if running under debugger, FALSE otherwise
 */
BOOL CheckTLSCallbacks();

/**
 * Check for SeDebugPrivilege (debugger often has this)
 * @return TRUE if SeDebugPrivilege present, FALSE otherwise
 */
BOOL CheckSeDebugPrivilege();

/**
 * Get current hardware breakpoint status
 * @param pDr0-3 Output for debug registers
 * @param pDr6 Output for DR6
 * @param pDr7 Output for DR7
 */
void GetHardwareBreakpointStatus(PVOID* pDr0, PVOID* pDr1, PVOID* pDr2, PVOID* pDr3, DWORD* pDr6, DWORD* pDr7);

/**
 * Clear hardware breakpoints
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ClearHardwareBreakpoints();