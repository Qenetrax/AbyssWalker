// Light Theme
import QtQuick
import "."

Theme {
    // Override with Light theme specific values
    primaryColor: "#0369A1"
    secondaryColor: "#0EA5E9"
    ctaColor: "#22C55E"
    backgroundColor: "#F0F9FF"
    surfaceColor: "#FFFFFF"
    textColor: "#0C4A6E"
    textSecondaryColor: "#64748B"
    borderColor: "#E2E8F0"

    // Agent status colors
    property color agentOnline: "#22C55E"
    property color agentOffline: "#94A3B8"
    property color agentDead: "#EF4444"

    // Listener status colors
    property color listenerActive: "#22C55E"
    property color listenerStopped: "#94A3B8"
    property color listenerError: "#EF4444"

    // Priority colors
    property color priorityHigh: "#EF4444"
    property color priorityMedium: "#F59E0B"
    property color priorityLow: "#22C55E"

    // OS colors
    property color osWindows: "#0078D4"
    property color osLinux: "#FCC624"
    property color osMacos: "#A2AAAD"
}
