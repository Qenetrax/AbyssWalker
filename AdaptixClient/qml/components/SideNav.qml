// Side Navigation Panel
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property string currentPage

    signal pageSelected(string page)

    color: theme.surfaceColor
    border.width: 1
    border.color: theme.borderColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        // Logo Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: theme.primaryColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                // Logo Icon
                Rectangle {
                    width: 32
                    height: 32
                    radius: 6
                    color: "#FFFFFF"

                    Text {
                        anchors.centerIn: parent
                        text: "A"
                        font.family: theme.fontMono
                        font.pixelSize: 18
                        font.bold: true
                        color: theme.primaryColor
                    }
                }

                Text {
                    text: "AdaptixC2"
                    font.family: theme.fontFamily
                    font.pixelSize: 18
                    font.bold: true
                    color: "#FFFFFF"
                }
            }
        }

        // Navigation Items
        ListView {
            id: navList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 8
            model: navModel
            delegate: navDelegate
            spacing: 2
            clip: true

            ListModel {
                id: navModel
                ListElement { name: "Dashboard"; page: "DashboardPage"; icon: "dashboard" }
                ListElement { name: "Agents"; page: "AgentsPage"; icon: "agent" }
                ListElement { name: "Listeners"; page: "ListenersPage"; icon: "listener" }
                ListElement { name: "Credentials"; page: "CredentialsPage"; icon: "key" }
                ListElement { name: "Targets"; page: "TargetsPage"; icon: "target" }
                ListElement { name: "Downloads"; page: "DownloadsPage"; icon: "download" }
                ListElement { name: "Tunnels"; page: "TunnelsPage"; icon: "tunnel" }
                ListElement { name: "Logs"; page: "LogsPage"; icon: "logs" }
                ListElement { name: "Settings"; page: "SettingsPage"; icon: "settings" }
            }

            Component {
                id: navDelegate

                Rectangle {
                    width: navList.width
                    height: 44
                    color: currentPage === page ? theme.primaryColor + "15" : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 12

                        // Icon placeholder
                        Rectangle {
                            width: 20
                            height: 20
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: {
                                    switch(icon) {
                                        case "dashboard": return "◉"
                                        case "agent": return "●"
                                        case "listener": return "◈"
                                        case "key": return "⚿"
                                        case "target": return "◎"
                                        case "download": return "↓"
                                        case "tunnel": return "⇄"
                                        case "logs": return "☰"
                                        case "settings": return "⚙"
                                        default: return "•"
                                    }
                                }
                                font.family: theme.fontMono
                                font.pixelSize: 14
                                color: currentPage === page ? theme.primaryColor : theme.textSecondaryColor
                            }
                        }

                        Text {
                            text: name
                            font.family: theme.fontFamily
                            font.pixelSize: theme.fontSizeM
                            font.weight: currentPage === page ? Font.Medium : Font.Normal
                            color: currentPage === page ? theme.primaryColor : theme.textColor
                        }
                    }

                    // Left indicator
                    Rectangle {
                        visible: currentPage === page
                        width: 3
                        height: parent.height
                        color: theme.primaryColor
                        anchors.left: parent.left
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true

                        onEntered: parent.color = theme.primaryColor + "10"
                        onExited: parent.color = currentPage === page ? theme.primaryColor + "15" : "transparent"
                        onClicked: root.pageSelected(page)
                    }
                }
            }
        }

        // Version Info
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: theme.backgroundColor

            Text {
                anchors.centerIn: parent
                text: "v1.2.0 Enhanced"
                font.family: theme.fontMono
                font.pixelSize: theme.fontSizeXS
                color: theme.textSecondaryColor
            }
        }
    }
}
