// Status Bar
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property int onlineAgents
    property int activeListeners

    color: theme.backgroundColor
    height: 28

    // Top border
    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: theme.borderColor
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 24

        // Agents Online
        RowLayout {
            spacing: 6

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: theme.successColor
            }

            Text {
                text: onlineAgents + " Agents Online"
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeXS
                color: theme.textSecondaryColor
            }
        }

        // Active Listeners
        RowLayout {
            spacing: 6

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: theme.secondaryColor
            }

            Text {
                text: activeListeners + " Listeners Active"
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeXS
                color: theme.textSecondaryColor
            }
        }

        // Spacer
        Item { Layout.fillWidth: true }

        // Memory Usage
        Text {
            text: "Memory: " + Backend.memoryUsage
            font.family: theme.fontMono
            font.pixelSize: theme.fontSizeXS
            color: theme.textSecondaryColor
        }

        // CPU Usage
        Text {
            text: "CPU: " + Backend.cpuUsage
            font.family: theme.fontMono
            font.pixelSize: theme.fontSizeXS
            color: theme.textSecondaryColor
        }

        // Time
        Text {
            id: timeText
            font.family: theme.fontMono
            font.pixelSize: theme.fontSizeXS
            color: theme.textSecondaryColor

            Timer {
                interval: 1000
                running: true
                repeat: true
                onTriggered: timeText.text = Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm:ss")
            }
        }
    }
}
