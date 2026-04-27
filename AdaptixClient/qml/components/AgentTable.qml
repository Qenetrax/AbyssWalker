// Agent Table Component
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property var model

    color: theme.surfaceColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Filter Bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: "Search agents..."
                font.family: theme.fontFamily
                font.pixelSize: theme.fontSizeS

                background: Rectangle {
                    color: theme.backgroundColor
                    radius: theme.radiusS
                    border.width: 1
                    border.color: theme.borderColor
                }
            }

            ComboBox {
                id: statusFilter
                model: ["All Status", "Online", "Offline", "Dead"]
                width: 120
            }

            ComboBox {
                id: osFilter
                model: ["All OS", "Windows", "Linux", "macOS"]
                width: 120
            }
        }

        // Table
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: theme.backgroundColor
            radius: theme.radiusM
            border.width: 1
            border.color: theme.borderColor

            TableView {
                id: tableView
                anchors.fill: parent
                anchors.margins: 1
                model: root.model
                clip: true

                columnWidthProvider: function(column) {
                    switch(column) {
                        case 0: return 80   // ID
                        case 1: return 150  // Hostname
                        case 2: return 100  // IP
                        case 3: return 100  // OS
                        case 4: return 80   // Arch
                        case 5: return 100  // User
                        case 6: return 80   // PID
                        case 7: return 100  // Last Checkin
                        case 8: return 80   // Status
                        case 9: return 120  // Actions
                        default: return 100
                    }
                }

                delegate: Rectangle {
                    implicitHeight: 44
                    color: row % 2 === 0 ? theme.surfaceColor : theme.backgroundColor

                    // Row hover effect
                    Rectangle {
                        anchors.fill: parent
                        color: theme.primaryColor + "08"
                        visible: mouseArea.containsMouse
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        // Column content based on column index
                        Text {
                            font.family: theme.fontMono
                            font.pixelSize: theme.fontSizeS
                            color: theme.textColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true

                            text: {
                                if (!model) return ""
                                switch(column) {
                                    case 0: return model.agentId
                                    case 1: return model.hostname
                                    case 2: return model.ip
                                    case 3: return model.os
                                    case 4: return model.arch
                                    case 5: return model.user
                                    case 6: return model.pid
                                    case 7: return model.lastCheckin
                                    default: return ""
                                }
                            }

                            visible: column < 8
                        }

                        // Status Badge
                        Rectangle {
                            visible: column === 8
                            width: statusText.implicitWidth + 16
                            height: 24
                            radius: 12

                            color: {
                                if (!model) return theme.textSecondaryColor
                                switch(model.status) {
                                    case "Online": return theme.successColor
                                    case "Offline": return theme.warningColor
                                    case "Dead": return theme.errorColor
                                    default: return theme.textSecondaryColor
                                }
                            }

                            Text {
                                id: statusText
                                anchors.centerIn: parent
                                text: model ? model.status : ""
                                font.family: theme.fontFamily
                                font.pixelSize: theme.fontSizeXS
                                color: "#FFFFFF"
                            }
                        }

                        // Actions
                        RowLayout {
                            visible: column === 9
                            spacing: 4

                            Repeater {
                                model: [
                                    { icon: "▶", action: "interact" },
                                    { icon: "📁", action: "files" },
                                    { icon: "⚙", action: "processes" },
                                    { icon: "✕", action: "kill" }
                                ]

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 4
                                    color: actionMouse.containsMouse ? theme.primaryColor + "20" : "transparent"

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.icon
                                        font.family: theme.fontMono
                                        font.pixelSize: 12
                                        color: theme.textColor
                                    }

                                    MouseArea {
                                        id: actionMouse
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        hoverEnabled: true
                                        onClicked: {
                                            var agentId = tableView.model.data(
                                                tableView.model.index(row, 0),
                                                Qt.UserRole + 1
                                            )
                                            Backend.executeAgentAction(agentId, modelData.action)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.RightButton
                        onClicked: contextMenu.popup()
                    }

                    Menu {
                        id: contextMenu
                        MenuItem { text: "Interact" }
                        MenuItem { text: "File Browser" }
                        MenuItem { text: "Process Browser" }
                        MenuItem { text: "Screenshot" }
                        MenuSeparator {}
                        MenuItem { text: "Kill Agent"; onTriggered: Backend.killAgent(model.agentId) }
                    }
                }
            }
        }
    }
}
