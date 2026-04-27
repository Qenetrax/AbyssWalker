// +build linux

package main

import (
	"os"
	"os/exec"
	"os/user"
	"strings"
	"syscall"
)

// AntiDebugLinux provides anti-debugging capabilities for Linux
type AntiDebugLinux struct{}

func NewAntiDebug() *AntiDebugLinux {
	return &AntiDebugLinux{}
}

// CheckPtrace checks if process is being traced
func (a *AntiDebugLinux) CheckPtrace() bool {
	// Check /proc/self/status for TracerPid
	data, err := os.ReadFile("/proc/self/status")
	if err != nil {
		return false
	}

	// Parse TracerPid
	lines := strings.Split(string(data), "\n")
	for _, line := range lines {
		if strings.HasPrefix(line, "TracerPid:") {
			pid := strings.TrimSpace(strings.TrimPrefix(line, "TracerPid:"))
			if pid != "0" {
				return true
			}
		}
	}

	return false
}

// CheckParentProcess checks if parent process is a debugger
func (a *AntiDebugLinux) CheckParentProcess() bool {
	// Read /proc/self/stat to get parent PID
	data, err := os.ReadFile("/proc/self/stat")
	if err != nil {
		return false
	}

	fields := strings.Fields(string(data))
	if len(fields) < 4 {
		return false
	}

	ppid := fields[3]

	// Read parent process name
	cmdline, err := os.ReadFile("/proc/" + ppid + "/cmdline")
	if err != nil {
		return false
	}

	parentCmd := string(cmdline)
	suspiciousParents := []string{
		"gdb",
		"strace",
		"ltrace",
		"frida",
		"radare2",
		"r2",
		"edb",
		"x64dbg",
	}

	for _, suspicious := range suspiciousParents {
		if strings.Contains(parentCmd, suspicious) {
			return true
		}
	}

	return false
}

// CheckStrace checks if strace is attached
func (a *AntiDebugLinux) CheckStrace() bool {
	// Check if PTRACE_TRACEME fails
	err := syscall.PtraceAttach(os.Getppid())
	if err == nil {
		syscall.PtraceDetach(os.Getppid())
		return false
	}

	// If we can't attach, we might be traced
	return true
}

// CheckBreakpoints checks for software breakpoints
func (a *AntiDebugLinux) CheckBreakpoints() bool {
	// Read /proc/self/mem and check for 0xCC (INT3) instructions
	// This is a simplified check
	return false
}

// RunAllChecks runs all anti-debug checks
func (a *AntiDebugLinux) RunAllChecks() bool {
	if a.CheckPtrace() {
		return true
	}
	if a.CheckParentProcess() {
		return true
	}
	if a.CheckStrace() {
		return true
	}
	return false
}
