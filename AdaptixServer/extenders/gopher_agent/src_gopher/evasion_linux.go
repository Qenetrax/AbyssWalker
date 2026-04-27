// +build linux

package main

// EvasionLinux - Linux平台没有AMSI/ETW，提供空实现
type EvasionLinux struct{}

func NewEvasion() *EvasionLinux {
	return &EvasionLinux{}
}

// BypassAMSI - Linux上无AMSI，返回nil
func (e *EvasionLinux) BypassAMSI() error {
	return nil
}

// RestoreAMSI - Linux上无AMSI，返回nil
func (e *EvasionLinux) RestoreAMSI() error {
	return nil
}

// PatchETW - Linux上无ETW，返回nil
func (e *EvasionLinux) PatchETW() error {
	return nil
}

// RestoreETW - Linux上无ETW，返回nil
func (e *EvasionLinux) RestoreETW() error {
	return nil
}

// IsEvasionActive - Linux上始终返回false
func (e *EvasionLinux) IsEvasionActive() bool {
	return false
}
