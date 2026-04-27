// +build darwin

package main

// EvasionDarwin - macOS平台没有AMSI/ETW，提供空实现
type EvasionDarwin struct{}

func NewEvasion() *EvasionDarwin {
	return &EvasionDarwin{}
}

// BypassAMSI - macOS上无AMSI，返回nil
func (e *EvasionDarwin) BypassAMSI() error {
	return nil
}

// RestoreAMSI - macOS上无AMSI，返回nil
func (e *EvasionDarwin) RestoreAMSI() error {
	return nil
}

// PatchETW - macOS上无ETW，返回nil
func (e *EvasionDarwin) PatchETW() error {
	return nil
}

// RestoreETW - macOS上无ETW，返回nil
func (e *EvasionDarwin) RestoreETW() error {
	return nil
}

// IsEvasionActive - macOS上始终返回false
func (e *EvasionDarwin) IsEvasionActive() bool {
	return false
}
