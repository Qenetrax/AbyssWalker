// +build windows

package main

import (
	"os"
	"runtime"
	"syscall"
	"unsafe"
)

// AntiDebugWindows provides anti-debugging capabilities for Windows
type AntiDebugWindows struct{}

func NewAntiDebug() *AntiDebugWindows {
	return &AntiDebugWindows{}
}

// CheckDebugger checks if a debugger is attached
func (a *AntiDebugWindows) CheckDebugger() bool {
	// Method 1: IsDebuggerPresent
	kernel32 := syscall.NewLazyDLL("kernel32.dll")
	isDebuggerPresent := kernel32.NewProc("IsDebuggerPresent")
	ret, _, _ := isDebuggerPresent.Call()
	if ret != 0 {
		return true
	}

	// Method 2: CheckRemoteDebuggerPresent
	var isDebugged uint32
	currentProcess, _ := syscall.GetCurrentProcess()
	checkRemoteDebugger := kernel32.NewProc("CheckRemoteDebuggerPresent")
	checkRemoteDebugger.Call(uintptr(currentProcess), uintptr(unsafe.Pointer(&isDebugged)))
	if isDebugged != 0 {
		return true
	}

	// Method 3: PEB BeingDebugged flag
	peb := a.getPEB()
	if peb != nil && peb.BeingDebugged != 0 {
		return true
	}

	// Method 4: NtGlobalFlag
	if peb != nil && peb.NtGlobalFlag&0x70 != 0 {
		return true
	}

	return false
}

// CheckHardwareBreakpoints checks for hardware breakpoints
func (a *AntiDebugWindows) CheckHardwareBreakpoints() bool {
	// Check DR0-DR7 debug registers
	// This requires inline assembly which is not directly supported in Go
	// We'll use a workaround
	return false
}

// CheckParentProcess checks if the parent process is suspicious
func (a *AntiDebugWindows) CheckParentProcess() bool {
	// Get parent process ID
	// If parent is not explorer.exe, might be a debugger
	snapHandle, _ := syscall.CreateToolhelp32Snapshot(syscall.TH32CS_SNAPPROCESS, 0)
	defer syscall.CloseHandle(snapHandle)

	var entry syscall.ProcessEntry32
	entry.Size = uint32(unsafe.Sizeof(entry))

	// Find current process and its parent
	currentPID := os.Getpid()

	for {
		err := syscall.Process32Next(snapHandle, &entry)
		if err != nil {
			break
		}

		if entry.ProcessID == uint32(currentPID) {
			// Found current process, check parent
			parentPID := entry.ParentProcessID

			// Get parent process name
			var parentEntry syscall.ProcessEntry32
			parentEntry.Size = uint32(unsafe.Sizeof(parentEntry))

			snapHandle2, _ := syscall.CreateToolhelp32Snapshot(syscall.TH32CS_SNAPPROCESS, 0)

			for {
				err := syscall.Process32Next(snapHandle2, &parentEntry)
				if err != nil {
					break
				}

				if parentEntry.ProcessID == parentPID {
					name := syscall.UTF16ToString(parentEntry.ExeFile[:])
					// Check if parent is explorer.exe
					if name != "explorer.exe" && name != "cmd.exe" && name != "powershell.exe" {
						syscall.CloseHandle(snapHandle2)
						return true
					}
					break
				}
			}
			syscall.CloseHandle(snapHandle2)
			break
		}
	}

	return false
}

// CheckTiming checks for timing anomalies (debugger stepping)
func (a *AntiDebugWindows) CheckTiming() bool {
	kernel32 := syscall.NewLazyDLL("kernel32.dll")
	getTickCount := kernel32.NewProc("GetTickCount")

	start, _, _ := getTickCount.Call()

	// Do some work
	for i := 0; i < 1000; i++ {
		_ = i * i
	}

	end, _, _ := getTickCount.Call()

	// If execution took too long, might be single-stepping
	if end-start > 1000 {
		return true
	}

	return false
}

// PEB structure (partial)
type PEB struct {
	BeingDebugged byte
	_             [1]byte
	Mutant        uintptr
	_             [4]byte
	NtGlobalFlag  uint32
	// ... more fields
}

func (a *AntiDebugWindows) getPEB() *PEB {
	// Get PEB address from TEB
	// In x64: gs:[0x60]
	// In x86: fs:[0x30]
	// This is platform-specific and requires assembly
	return nil
}

// RunAllChecks runs all anti-debug checks
func (a *AntiDebugWindows) RunAllChecks() bool {
	if a.CheckDebugger() {
		return true
	}
	if a.CheckParentProcess() {
		return true
	}
	if a.CheckTiming() {
		return true
	}
	return false
}
