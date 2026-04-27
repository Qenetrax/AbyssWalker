#pragma once

/*
 * StackSpoof.h - Call Stack Spoofing
 *
 * Provides functionality to spoof the call stack when making
 * sensitive API calls, making them appear to originate from
 * legitimate Windows modules.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Stack frame information
typedef struct _STACK_FRAME {
    PVOID returnAddress;
    PVOID frameAddress;
} STACK_FRAME, *PSTACK_FRAME;

// Spoofing configuration
typedef struct _SPOOF_CONFIG {
    PVOID pTargetFunction;      // Function to call
    PVOID pSpoofReturn;         // Fake return address
    PVOID pRealReturn;          // Real return address
    PVOID pTrampoline;          // Trampoline address
    PVOID pStackBackup;         // Backup of original stack
    SIZE_T stackSize;           // Size of stack backup
} SPOOF_CONFIG, *PSPOOF_CONFIG;

// Common legitimate return addresses (from system DLLs)
typedef struct _LEGITIMATE_ADDRESS {
    PVOID address;
    char moduleName[64];
    char functionName[64];
} LEGITIMATE_ADDRESS, *PLEGITIMATE_ADDRESS;

// Function declarations

/**
 * Spoof call stack for a function call
 * @param targetFunc Function to call
 * @param spoofReturn Fake return address to use
 * @param args Function arguments
 * @return Result of the function call
 */
PVOID SpoofCall(PVOID targetFunc, PVOID spoofReturn, ...);

/**
 * Initialize stack spoofing system
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InitStackSpoof();

/**
 * Get a legitimate return address from a system module
 * @param moduleName Name of module (e.g., "kernel32.dll")
 * @return Return address within module, or NULL
 */
PVOID GetLegitimateReturnAddress(LPCSTR moduleName);

/**
 * Get return address from kernel32 (commonly used)
 * @return Return address in kernel32, or NULL
 */
PVOID GetKernel32ReturnAddress();

/**
 * Get return address from ntdll
 * @return Return address in ntdll, or NULL
 */
PVOID GetNtdllReturnAddress();

/**
 * Get return address from a random system DLL
 * @return Return address in a system DLL, or NULL
 */
PVOID GetRandomSystemReturnAddress();

/**
 * Execute function with spoofed stack
 * @param pConfig Spoofing configuration
 * @return Result of function execution
 */
PVOID ExecuteSpoofed(PSPOOF_CONFIG pConfig);

/**
 * Backup current stack state
 * @param pConfig Configuration to store backup
 * @return TRUE if successful, FALSE otherwise
 */
BOOL BackupStack(PSPOOF_CONFIG pConfig);

/**
 * Restore stack state from backup
 * @param pConfig Configuration containing backup
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RestoreStack(PSPOOF_CONFIG pConfig);

/**
 * Find function in module for return address
 * @param hModule Module handle
 * @param minOffset Minimum offset from module base
 * @param maxOffset Maximum offset from module base
 * @return Suitable return address
 */
PVOID FindSuitableReturnAddress(HMODULE hModule, DWORD minOffset, DWORD maxOffset);

/**
 * Call function with fake call stack
 * @param pFunc Function to call
 * @param numArgs Number of arguments
 * @param ... Function arguments
 * @return Function result
 */
typedef PVOID(*SpoofedCall_t)(PVOID, PVOID, SIZE_T, ...);
extern SpoofedCall_t g_pSpoofedCall;

// Assembly stub declarations
extern "C" {
    PVOID SpoofCallStub(PVOID targetFunc, PVOID spoofReturn, PVOID stackFrame, PVOID* args);
    PVOID SpoofCallInternal();
}

// Helper macros for easy stack spoofing
#define SPOOF_CALL(func, retAddr, ...) \
    SpoofCall((PVOID)func, retAddr, ##__VA_ARGS__)

#define SPOOF_K32_CALL(func, ...) \
    SpoofCall((PVOID)func, GetKernel32ReturnAddress(), ##__VA_ARGS__)

#define SPOOF_NTDLL_CALL(func, ...) \
    SpoofCall((PVOID)func, GetNtdllReturnAddress(), ##__VA_ARGS__)