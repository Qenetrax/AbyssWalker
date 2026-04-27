import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // Design System Colors
    readonly property color primaryColor: "#0369A1"
    readonly property color secondaryColor: "#0EA5E9"
    readonly property color ctaColor: "#22C55E"
    readonly property color backgroundColor: "#F0F9FF"
    readonly property color surfaceColor: "#FFFFFF"
    readonly property color textColor: "#0C4A6E"
    readonly property color textSecondaryColor: "#64748B"
    readonly property color borderColor: "#E2E8F0"
    readonly property color successColor: "#22C55E"
    readonly property color warningColor: "#F59E0B"
    readonly property color dangerColor: "#EF4444"

    property string filterText: ""

    // Signals
    signal createTunnel(string agentId)
    signal stopTunnel(string tunnelId)
    signal removeTunnel(string tunnelId)

    Rectangle {
        anchors.fill: parent
        color: backgroundColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Toolbar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                color: surfaceColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    Text {
                        text: qsTr("Tunnels")
                        font.family: "Fira Sans"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: textColor
                    }

                    // Separator
                    Rectangle {
                        width: 1
                        height: 32
                        color: borderColor
                    }

                    // Search
                    TextField {
                        id: searchField
                        Layout.preferredWidth: 280
                        Layout.preferredHeight: 36
                        placeholderText: qsTr("Search tunnels...")
                        font.family: "Fira Sans"
                        font.pixelSize: 14
                        color: textColor
                        onTextChanged: filterText = text

                        background: Rectangle {
                            color: backgroundColor
                            radius: 6
                            border.color: parent.activeFocus ? primaryColor : borderColor
                            border.width: 1
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Statistics
                    RowLayout {
                        spacing: 16

                        Rectangle {
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 36
                            color: backgroundColor
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: successColor
                                }

                                Text {
                                    text: qsTr("Active: %1").arg(activeCount)
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                }
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 36
                            color: backgroundColor
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: dangerColor
                                }

                                Text {
                                    text: qsTr("Stopped: %1").arg(stoppedCount)
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                }
                            }
                        }
                    }
                }

                // Bottom border
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: borderColor
                }
            }

            // Tunnels List
            ListView {
                id: tunnelsList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: tunnelsModel
                spacing: 8
                leftMargin: 16
                rightMargin: 16
                topMargin: 16
                bottomMargin: 16

                delegate: Rectangle {
                    width: tunnelsList.width - tunnelsList.leftMargin - tunnelsList.rightMargin
                    height: 120
                    color: surfaceColor
                    radius: 12
                    border.color: borderColor
                    border.width: 1

                    Rectangle {
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 4
                        radius: 2
                        color: model.active ? successColor : dangerColor
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 16

                        // Type Icon
                        Rectangle {
                            Layout.preferredWidth: 56
                            Layout.preferredHeight: 56
                            radius: 12
                            color: Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.1)

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 2

                                Text {
                                    text: model.tunnelType
                                    font.family: "Fira Code"
                                    font.pixelSize: 14
                                    font.weight: Font.Bold
                                    color: primaryColor
                                    Layout.alignment: Qt.AlignHCenter
                                }

                                Text {
                                    text: model.port.toString()
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }

                        // Info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            RowLayout {
                                spacing: 8

                                Text {
                                    text: model.description || model.tunnelType
                                    font.family: "Fira Sans"
                                    font.pixelSize: 16
                                    font.weight: Font.DemiBold
                                    color: textColor
                                }

                                Rectangle {
                                    visible: model.active
                                    width: statusText.width + 16
                                    height: 22
                                    radius: 11
                                    color: Qt.rgba(successColor.r, successColor.g, successColor.b, 0.1)

                                    Text {
                                        id: statusText
                                        anchors.centerIn: parent
                                        text: qsTr("Active")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 11
                                        font.weight: Font.Medium
                                        color: successColor
                                    }
                                }

                                Rectangle {
                                    visible: !model.active
                                    width: statusText2.width + 16
                                    height: 22
                                    radius: 11
                                    color: Qt.rgba(dangerColor.r, dangerColor.g, dangerColor.b, 0.1)

                                    Text {
                                        id: statusText2
                                        anchors.centerIn: parent
                                        text: qsTr("Stopped")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 11
                                        font.weight: Font.Medium
                                        color: dangerColor
                                    }
                                }
                            }

                            GridLayout {
                                columns: 4
                                columnSpacing: 24
                                rowSpacing: 4

                                Text {
                                    text: qsTr("Agent:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }
                                Text {
                                    text: model.agentId
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textColor
                                }

                                Text {
                                    text: qsTr("Bind Address:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }
                                Text {
                                    text: model.bindAddress + ":" + model.port
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textColor
                                }

                                Text {
                                    text: qsTr("Target:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }
                                Text {
                                    text: model.target || "-"
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textColor
                                }

                                Text {
                                    text: qsTr("Connections:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }
                                Text {
                                    text: model.connections || "0"
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textColor
                                }
                            }
                        }

                        // Action Buttons
                        RowLayout {
                            spacing: 8

                            Button {
                                visible: model.active
                                text: qsTr("Stop")
                                font.family: "Fira Sans"
                                onClicked: root.stopTunnel(model.tunnelId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
                                    color: parent.down ? Qt.darker(dangerColor, 1.1) : dangerColor
                                    radius: 6
                                }

                                contentItem: Text {
                                    text: parent.text
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            Button {
                                visible: !model.active
                                text: qsTr("Start")
                                font.family: "Fira Sans"
                                onClicked: root.createTunnel(model.agentId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
                                    color: parent.down ? Qt.darker(ctaColor, 1.1) : ctaColor
                                    radius: 6
                                }

                                contentItem: Text {
                                    text: parent.text
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            Button {
                                text: qsTr("Remove")
                                font.family: "Fira Sans"
                                onClicked: root.removeTunnel(model.tunnelId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
                                    color: parent.down ? Qt.darker(surfaceColor, 1.1) : surfaceColor
                                    radius: 6
                                    border.color: borderColor
                                    border.width: 1
                                }

                                contentItem: Text {
                                    text: parent.text
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                }

                // Empty state
                Text {
                    anchors.centerIn: parent
                    visible: tunnelsModel.count === 0
                    text: qsTr("No tunnels configured")
                    font.family: "Fira Sans"
                    font.pixelSize: 16
                    color: textSecondaryColor
                }
            }
        }
    }

    // Tunnels Model
    ListModel {
        id: tunnelsModel
    }

    property int activeCount: 0
    property int stoppedCount: 0

    // Public API
    function addTunnel(tunnelData) {
        tunnelsModel.append(tunnelData)
        updateCounts()
    }

    function updateTunnel(tunnelId, tunnelData) {
        for (var i = 0; i < tunnelsModel.count; i++) {
            if (tunnelsModel.get(i).tunnelId === tunnelId) {
                tunnelsModel.set(i, tunnelData)
                break
            }
        }
        updateCounts()
    }

    function removeTunnelById(tunnelId) {
        for (var i = 0; i < tunnelsModel.count; i++) {
            if (tunnelsModel.get(i).tunnelId === tunnelId) {
                tunnelsModel.remove(i)
                break
            }
        }
        updateCounts()
    }

    function clearTunnels() {
        tunnelsModel.clear()
        activeCount = 0
        stoppedCount = 0
    }

    function updateCounts() {
        var active = 0
        var stopped = 0
        for (var i = 0; i < tunnelsModel.count; i++) {
            if (tunnelsModel.get(i).active) {
                active++
            } else {
                stopped++
            }
        }
        activeCount = active
        stoppedCount = stopped
    }
}