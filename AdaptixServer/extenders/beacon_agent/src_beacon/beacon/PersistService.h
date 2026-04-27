#pragma once

/*
 * PersistService.h - Windows Service Persistence
 *
 * Provides Windows Service-based persistence mechanisms.
 * WARNING: Use only with explicit user consent for legitimate purposes.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Service configuration
typedef struct _SERVICE_PERSIST_CONFIG {
    LPCSTR serviceName;             // Service name (internal)
    LPCSTR displayName;             // Display name
    LPCSTR description;             // Service description
    LPCSTR targetPath;              // Executable path
    LPCSTR arguments;               // Command line arguments
    DWORD startType;                // SERVICE_AUTO_START, etc.
    BOOL bRequireAdmin;             // Requires admin (always TRUE for services)
} SERVICE_PERSIST_CONFIG, *PSERVICE_PERSIST_CONFIG;

// Function declarations

/**
 * Install Windows Service persistence
 * @param serviceName Internal service name
 * @param displayName Display name
 * @param targetPath Executable path
 * @param startType Service start type
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallServicePersist(LPCSTR serviceName, LPCSTR displayName, LPCSTR targetPath, DWORD startType);

/**
 * Install Windows Service with full configuration
 * @param pConfig Service configuration
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallServicePersistEx(PSERVICE_PERSIST_CONFIG pConfig);

/**
 * Remove Windows Service
 * @param serviceName Service name to remove
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RemoveServicePersist(LPCSTR serviceName);

/**
 * Check if service exists
 * @param serviceName Service name
 * @return TRUE if exists, FALSE otherwise
 */
BOOL CheckServiceExist(LPCSTR serviceName);

/**
 * Start installed service
 * @param serviceName Service name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL StartServicePersist(LPCSTR serviceName);

/**
 * Stop service
 * @param serviceName Service name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL StopServicePersist(LPCSTR serviceName);

/**
 * Query service status
 * @param serviceName Service name
 * @param pStatus Output status
 * @return TRUE if successful, FALSE otherwise
 */
BOOL QueryServiceStatusPersist(LPCSTR serviceName, SERVICE_STATUS* pStatus);

/**
 * Set service description
 * @param hService Service handle
 * @param description Description text
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetServiceDescription(HANDLE hService, LPCSTR description);

/**
 * Configure service failure actions (restart on failure)
 * @param serviceName Service name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ConfigureServiceRecovery(LPCSTR serviceName);