#pragma once

/*
 * AntiSandbox.h - Anti-Sandbox/Anti-VM Techniques
 *
 * Provides techniques to detect and evade sandboxes,
 * virtual machines, and analysis environments.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Sandbox check method flags
typedef enum _SANDBOX_CHECK {
    SANDBOX_CHECK_ENVIRONMENT    = 0x0001,
    SANDBOX_CHECK_VM             = 0x0002,
    SANDBOX_CHECK_ANALYSIS       = 0x0004,
    SANDBOX_CHECK_TIMING         = 0x0008,
    SANDBOX_CHECK_HARDWARE       = 0x0010,
    SANDBOX_CHECK_NETWORK        = 0x0020,
    SANDBOX_CHECK_MEMORY         = 0x0040,
    SANDBOX_CHECK_PROCESSES      = 0x0080,
    SANDBOX_CHECK_USER           = 0x0100,
    SANDBOX_CHECK_ALL            = 0xFFFF
} SANDBOX_CHECK;

// Sandbox detection result
typedef struct _SANDBOX_CHECK_RESULT {
    BOOL bSandboxDetected;
    BOOL bVMDetected;
    BOOL bAnalysisDetected;
    DWORD detectedMethods;
    CHAR details[512];
} SANDBOX_CHECK_RESULT, *PSANDBOX_CHECK_RESULT;

// Function declarations

/**
 * Check sandbox environment variables
 * @return TRUE if sandbox environment detected, FALSE otherwise
 */
BOOL CheckSandboxEnvironment();

/**
 * Check for common VM indicators
 * @return TRUE if VM detected, FALSE otherwise
 */
BOOL CheckVM();

/**
 * Check for analysis tools
 * @return TRUE if analysis tools detected, FALSE otherwise
 */
BOOL CheckAnalysisTools();

/**
 * Timing-based sandbox detection
 * @return TRUE if timing indicates sandbox, FALSE otherwise
 */
BOOL CheckTimingSandbox();

/**
 * Check hardware characteristics
 * @return TRUE if hardware indicates VM, FALSE otherwise
 */
BOOL CheckHardwareVM();

/**
 * Check network configuration
 * @return TRUE if network indicates sandbox, FALSE otherwise
 */
BOOL CheckNetworkSandbox();

/**
 * Check memory characteristics
 * @return TRUE if memory indicates sandbox, FALSE otherwise
 */
BOOL CheckMemorySandbox();

/**
 * Check for sandbox processes
 * @return TRUE if sandbox processes found, FALSE otherwise
 */
BOOL CheckSandboxProcesses();

/**
 * Check user activity
 * @return TRUE if low user activity (sandbox), FALSE otherwise
 */
BOOL CheckUserActivity();

/**
 * Comprehensive sandbox check
 * @param checks Bitmask of checks to perform
 * @param pResult Output result structure
 * @return TRUE if sandbox detected, FALSE otherwise
 */
BOOL CheckSandbox(DWORD checks, PSANDBOX_CHECK_RESULT pResult);

/**
 * Simple sandbox check
 * @return TRUE if sandbox detected, FALSE otherwise
 */
BOOL IsSandbox();

/**
 * Check CPU features for VM indicators
 * @return TRUE if VM CPU features found, FALSE otherwise
 */
BOOL CheckCPUFeatures();

/**
 * Check for VMware specific artifacts
 * @return TRUE if VMware detected, FALSE otherwise
 */
BOOL CheckVMware();

/**
 * Check for VirtualBox specific artifacts
 * @return TRUE if VirtualBox detected, FALSE otherwise
 */
BOOL CheckVirtualBox();

/**
 * Check for Hyper-V specific artifacts
 * @return TRUE if Hyper-V detected, FALSE otherwise
 */
BOOL CheckHyperV();

/**
 * Check for QEMU specific artifacts
 * @return TRUE if QEMU detected, FALSE otherwise
 */
BOOL CheckQEMU();

/**
 * Check for KVM specific artifacts
 * @return TRUE if KVM detected, FALSE otherwise
 */
BOOL CheckKVM();

/**
 * Check for Parallels specific artifacts
 * @return TRUE if Parallels detected, FALSE otherwise
 */
BOOL CheckParallels();

/**
 * Check for Xen specific artifacts
 * @return TRUE if Xen detected, FALSE otherwise
 */
BOOL CheckXen();

/**
 * Check for WINE environment
 * @return TRUE if WINE detected, FALSE otherwise
 */
BOOL CheckWine();

/**
 * Check screen resolution (often low in sandboxes)
 * @return TRUE if suspicious resolution, FALSE otherwise
 */
BOOL CheckScreenResolution();

/**
 * Check last input time
 * @param thresholdMs Threshold in milliseconds
 * @return TRUE if idle too long (sandbox), FALSE otherwise
 */
BOOL CheckLastInputTime(DWORD thresholdMs);

/**
 * Check mouse movement
 * @return TRUE if mouse never moved (sandbox), FALSE otherwise
 */
BOOL CheckMouseMovement();

/**
 * Check number of processes
 * @param minProcesses Minimum expected processes
 * @return TRUE if too few processes (sandbox), FALSE otherwise
 */
BOOL CheckProcessCount(DWORD minProcesses);

/**
 * Check disk size
 * @param minSizeGB Minimum expected disk size in GB
 * @return TRUE if disk too small (sandbox), FALSE otherwise
 */
BOOL CheckDiskSize(DWORD minSizeGB);

/**
 * Check number of CPUs
 * @param minCpus Minimum expected CPUs
 * @return TRUE if too few CPUs (sandbox), FALSE otherwise
 */
BOOL CheckCpuCount(DWORD minCpus);

/**
 * Check physical memory
 * @param minMemoryMB Minimum expected memory in MB
 * @return TRUE if too little memory (sandbox), FALSE otherwise
 */
BOOL CheckPhysicalMemory(DWORD minMemoryMB);

/**
 * Check for sleep acceleration
 * @return TRUE if sleep is accelerated (sandbox), FALSE otherwise
 */
BOOL CheckSleepAcceleration();