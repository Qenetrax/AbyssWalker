// +build darwin

package main

import (
	"os"
	"os/exec"
	"strings"
	"syscall"
	"unsafe"
)

// AntiDebugDarwin provides anti-debugging capabilities for macOS
type AntiDebugDarwin struct{}

func NewAntiDebug() *AntiDebugDarwin {
	return &AntiDebugDarwin{}
}

// CheckPtrace checks if process is being traced
func (a *AntiDebugDarwin) CheckPtrace() bool {
	// On macOS, use sysctl to check P_TRACED flag
	type KInfoProc struct {
		Kp_proc struct {
			P_flag int32
		}
		// ... other fields
	}

	var info KInfoProc
	var size uint32 = uint32(unsafe.Sizeof(info))

	name := [4]int32{1, 14, 1, 0} // CTL_KERN, KERN_PROC, KERN_PROC_PID

	_, _, errno := syscall.Syscall6(
		syscall.SYS___SYSCTL,
		uintptr(unsafe.Pointer(&name[0])),
		4,
		uintptr(unsafe.Pointer(&info)),
		uintptr(unsafe.Pointer(&size)),
		0, 0,
	)

	if errno != 0 {
		return false
	}

	// P_TRACED flag is 0x00000800
	if info.Kp_proc.P_flag&0x00000800 != 0 {
		return true
	}

	return false
}

// CheckTaskForPort checks if task_for_pid is possible (debugger indicator)
func (a *AntiDebugDarwin) CheckTaskForPort() bool {
	// Try to get task port for our own process
	// If we can't, we might be debugged
	var task uint32
	ret := task_for_pid(mach_task_self(), int32(os.Getpid()), &task)
	return ret != 0
}

// CheckParentProcess checks if parent is a debugger
func (a *AntiDebugDarwin) CheckParentProcess() bool {
	ppid := os.Getppid()

	// Get parent process name
	cmd := exec.Command("ps", "-p", string(ppid), "-o", "comm=")
	output, err := cmd.Output()
	if err != nil {
		return false
	}

	parentName := strings.TrimSpace(string(output))
	suspiciousParents := []string{
		"lldb",
		"gdb",
		"dtrace",
		"frida",
		"ida",
		"hopper",
		"radare2",
		"x64dbg",
	}

	for _, suspicious := range suspiciousParents {
		if strings.Contains(parentName, suspicious) {
			return true
		}
	}

	return false
}

// CheckAmfi checks if AMFI (Apple Mobile File Integrity) is debugging
func (a *AntiDebugDarwin) CheckAmfi() bool {
	// Check for DYLD_INSERT_LIBRARIES
	if os.Getenv("DYLD_INSERT_LIBRARIES") != "" {
		return true
	}
	return false
}

// RunAllChecks runs all anti-debug checks
func (a *AntiDebugDarwin) RunAllChecks() bool {
	if a.CheckPtrace() {
		return true
	}
	if a.CheckTaskForPort() {
		return true
	}
	if a.CheckParentProcess() {
		return true
	}
	if a.CheckAmfi() {
		return true
	}
	return false
}

// Mach task functions (simplified)
func mach_task_self() int32 {
	return 0 // Simplified
}

func task_for_pid(task int32, pid int32, targetTask *uint32) int32 {
	return 0 // Simplified - would use Mach API
}
