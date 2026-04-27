// Top Bar
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property bool connected
    property string serverUrl
    property string currentUser

    color: theme.surfaceColor
    border.width: 0
    border.color: theme.borderColor

    // Bottom border
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: theme.borderColor
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: 16

        // Connection Status
        RowLayout {
            spacing: 8

            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: connected ? theme.successColor : theme.errorColor

                // Pulse animation when connected
                SequentialAnimation on scale {
                    running: connected
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.2; duration: 1000 }
                    NumberAnimation { to: 1.0; duration: 1000 }
                }
            }

            Text {
                text: connected ? "Connected" : "Disconnected"
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeS
                color: connected ? theme.successColor : theme.errorColor
            }
        }

        // Separator
        Rectangle {
            width: 1
            height: 24
            color: theme.borderColor
        }

        // Server URL
        Text {
            text: serverUrl || "Not connected"
            font.family: theme.fontMono
            font.pixelSize: theme.fontSizeS
            color: theme.textSecondaryColor
        }

        // Spacer
        Item { Layout.fillWidth: true }

        // Quick Actions
        RowLayout {
            spacing: 8

            // Build Agent Button
            Button {
                text: "Build Agent"
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeS

                background: Rectangle {
                    color: parent.down ? theme.primaryColor + "CC" : parent.hovered ? theme.primaryColor + "DD" : theme.primaryColor
                    radius: theme.radiusS
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    var component = Qt.createComponent("qrc:/qml/dialogs/AgentBuilderDialog.qml")
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(root)
                        dialog.open()
                    }
                }
            }

            // Start Listener Button
            Button {
                text: "Start Listener"
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeS

                background: Rectangle {
                    color: parent.down ? theme.secondaryColor + "CC" : parent.hovered ? theme.secondaryColor + "DD" : theme.secondaryColor
                    radius: theme.radiusS
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    var component = Qt.createComponent("qrc:/qml/dialogs/ListenerDialog.qml")
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(root)
                        dialog.open()
                    }
                }
            }
        }

        // Separator
        Rectangle {
            width: 1
            height: 24
            color: theme.borderColor
        }

        // User Menu
        RowLayout {
            spacing: 8

            Rectangle {
                width: 32
                height: 32
                radius: 16
                color: theme.primaryColor

                Text {
                    anchors.centerIn: parent
                    text: currentUser ? currentUser.charAt(0).toUpperCase() : "?"
                    font.family: theme.fontFamily
                    font.pixelSize: theme.fontSizeM
                    font.bold: true
                    color: "#FFFFFF"
                }
            }

            Text {
                text: currentUser || "Not logged in"
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeS
                color: theme.textColor
            }

            // Dropdown indicator
            Text {
                text: "▼"
                font.family: theme.fontMono
                font.pixelSize: 8
                color: theme.textSecondaryColor
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: userMenu.open()
            }

            Menu {
                id: userMenu
                MenuItem {
                    text: "Settings"
                    onTriggered: console.log("Settings")
                }
                MenuItem {
                    text: "Disconnect"
                    onTriggered: Backend.disconnect()
                }
            }
        }
    }
}
