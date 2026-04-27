// +build windows

package main

import (
	"os"
	"os/user"
	"runtime"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

// AntiSandboxWindows provides anti-sandbox capabilities for Windows
type AntiSandboxWindows struct{}

func NewAntiSandbox() *AntiSandboxWindows {
	return &AntiSandboxWindows{}
}

// CheckUsername checks for common sandbox usernames
func (a *AntiSandboxWindows) CheckUsername() bool {
	sandboxUsers := []string{
		"malware",
		"sample",
		"virus",
		"sandbox",
		"vmware",
		"vbox",
		"currentuser",
		"user",
		"test",
	}

	currentUser, err := user.Current()
	if err != nil {
		return false
	}

	username := strings.ToLower(currentUser.Username)
	for _, sandbox := range sandboxUsers {
		if strings.Contains(username, sandbox) {
			return true
		}
	}

	return false
}

// CheckComputerName checks for common sandbox computer names
func (a *AntiSandboxWindows) CheckComputerName() bool {
	sandboxNames := []string{
		"sample",
		"virus",
		"sandbox",
		"malware",
		"vmware",
		"virtual",
		"vbox",
	}

	hostname, err := os.Hostname()
	if err != nil {
		return false
	}

	hostname = strings.ToLower(hostname)
	for _, sandbox := range sandboxNames {
		if strings.Contains(hostname, sandbox) {
			return true
		}
	}

	return false
}

// CheckEnvironment checks for sandbox environment variables
func (a *AntiSandboxWindows) CheckEnvironment() bool {
	sandboxEnvVars := []string{
		"SAMPLEPATH",
		"SANDBOX_PATH",
		"VBOX_INSTALL_PATH",
		"VMWARE_PATH",
	}

	for _, envVar := range sandboxEnvVars {
		if os.Getenv(envVar) != "" {
			return true
		}
	}

	return false
}

// CheckMemory checks for low memory (common in sandboxes)
func (a *AntiSandboxWindows) CheckMemory() bool {
	kernel32 := syscall.NewLazyDLL("kernel32.dll")
	globalMemoryStatusEx := kernel32.NewProc("GlobalMemoryStatusEx")

	type MEMORYSTATUSEX struct {
		DwLength                uint32
		DwMemoryLoad            uint32
		UllTotalPhys            uint64
		UllAvailPhys            uint64
		UllTotalPageFile        uint64
		UllAvailPageFile        uint64
		UllTotalVirtual         uint64
		UllAvailVirtual         uint64
		UllAvailExtendedVirtual uint64
	}

	var memStatus MEMORYSTATUSEX
	memStatus.DwLength = uint32(unsafe.Sizeof(memStatus))

	globalMemoryStatusEx.Call(uintptr(unsafe.Pointer(&memStatus)))

	// If total physical memory is less than 2GB, likely a sandbox
	if memStatus.UllTotalPhys < 2*1024*1024*1024 {
		return true
	}

	return false
}

// CheckCores checks for single CPU core (common in sandboxes)
func (a *AntiSandboxWindows) CheckCores() bool {
	if runtime.NumCPU() < 2 {
		return true
	}
	return false
}

// CheckUptime checks for low system uptime (common in sandboxes)
func (a *AntiSandboxWindows) CheckUptime() bool {
	kernel32 := syscall.NewLazyDLL("kernel32.dll")
	getTickCount64 := kernel32.NewProc("GetTickCount64")

	ticks, _, _ := getTickCount64.Call()
	uptime := time.Duration(ticks) * time.Millisecond

	// If uptime is less than 10 minutes, likely a sandbox
	if uptime < 10*time.Minute {
		return true
	}

	return false
}

// CheckMouseMovement checks for recent mouse activity
func (a *AntiSandboxWindows) CheckMouseMovement() bool {
	user32 := syscall.NewLazyDLL("user32.dll")
	getLastInputInfo := user32.NewProc("GetLastInputInfo")

	type LASTINPUTINFO struct {
		CbSize    uint32
		DwTime    uint32
	}

	var lii LASTINPUTINFO
	lii.CbSize = uint32(unsafe.Sizeof(lii))

	getLastInputInfo.Call(uintptr(unsafe.Pointer(&lii)))

	kernel32 := syscall.NewLazyDLL("kernel32.dll")
	getTickCount := kernel32.NewProc("GetTickCount")

	ticks, _, _ := getTickCount.Call()
	idleTime := uint32(ticks) - lii.DwTime

	// If idle time is very high, might be a sandbox
	if idleTime > 300000 { // 5 minutes
		return true
	}

	return false
}

// CheckRegistry checks for virtualization artifacts in registry
func (a *AntiSandboxWindows) CheckRegistry() bool {
	// Check for VMware registry keys
	vmwareKeys := []string{
		`SYSTEM\CurrentControlSet\Services\vmicheartbeat`,
		`SYSTEM\CurrentControlSet\Services\vmicvss`,
		`SYSTEM\CurrentControlSet\Services\VMTools`,
	}

	for _, key := range vmwareKeys {
		keyPtr, err := syscall.UTF16PtrFromString(key)
		if err != nil {
			continue
		}

		var hKey syscall.Handle
		err = syscall.RegOpenKeyEx(syscall.HKEY_LOCAL_MACHINE, keyPtr, 0, syscall.KEY_READ, &hKey)
		if err == nil {
			syscall.RegCloseKey(hKey)
			return true
		}
	}

	return false
}

// RunAllChecks runs all anti-sandbox checks
func (a *AntiSandboxWindows) RunAllChecks() bool {
	if a.CheckUsername() {
		return true
	}
	if a.CheckComputerName() {
		return true
	}
	if a.CheckEnvironment() {
		return true
	}
	if a.CheckMemory() {
		return true
	}
	if a.CheckCores() {
		return true
	}
	if a.CheckUptime() {
		return true
	}
	if a.CheckMouseMovement() {
		return true
	}
	if a.CheckRegistry() {
		return true
	}
	return false
}
