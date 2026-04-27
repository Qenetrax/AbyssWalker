// Terminal View Component
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property string agentId
    property var commandHistory: []

    color: "#1E1E1E"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Terminal Output
        ScrollView {
            id: outputScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                id: outputArea
                readOnly: true
                font.family: "Fira Code"
                font.pixelSize: 13
                color: "#D4D4D4"
                selectionColor: "#264F78"
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                text: "AdaptixC2 Terminal\nType 'help' for available commands.\n\n"

                background: Rectangle {
                    color: "#1E1E1E"
                }
            }
        }

        // Command Input
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#252526"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text {
                    text: ">"
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: "#22C55E"
                }

                TextField {
                    id: commandInput
                    Layout.fillWidth: true
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: "#D4D4D4"
                    placeholderText: "Enter command..."
                    placeholderTextColor: "#6A6A6A"

                    background: Rectangle {
                        color: "transparent"
                    }

                    onAccepted: {
                        if (text.trim() !== "") {
                            executeCommand(text)
                            text = ""
                        }
                    }

                    Keys.onUpPressed: {
                        // Navigate command history up
                    }

                    Keys.onDownPressed: {
                        // Navigate command history down
                    }
                }
            }
        }
    }

    function executeCommand(cmd) {
        outputArea.text += "\n> " + cmd + "\n"
        commandHistory.push(cmd)

        Backend.executeCommand(agentId, cmd, function(result) {
            outputArea.text += result + "\n"
            // Auto scroll to bottom
            outputScroll.ScrollBar.vertical.position = 1.0
        })
    }

    function appendOutput(text) {
        outputArea.text += text
        outputScroll.ScrollBar.vertical.position = 1.0
    }

    function clearOutput() {
        outputArea.text = "AdaptixC2 Terminal\nType 'help' for available commands.\n\n"
    }
}
