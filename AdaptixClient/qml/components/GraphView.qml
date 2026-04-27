// Graph View Component - Agent Topology
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property var model

    color: theme.backgroundColor

    // Canvas for drawing connections
    Canvas {
        id: connectionCanvas
        anchors.fill: parent
        z: 0

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = theme.borderColor
            ctx.lineWidth = 2

            // Draw connections between nodes
            // This will be populated from the model
        }
    }

    // Agent Nodes
    Repeater {
        model: root.model

        Rectangle {
            id: agentNode
            width: 180
            height: 100
            radius: theme.radiusM
            color: theme.surfaceColor
            border.width: 2
            border.color: {
                switch(model.status) {
                    case "Online": return theme.successColor
                    case "Offline": return theme.warningColor
                    case "Dead": return theme.errorColor
                    default: return theme.borderColor
                }
            }

            x: model.graphX || 0
            y: model.graphY || 0

            // Drag handler
            MouseArea {
                anchors.fill: parent
                drag.target: parent
                drag.axis: Drag.XAndYAxis
                cursorShape: Qt.SizeAllCursor

                onReleased: {
                    // Update model position
                }

                onDoubleClicked: {
                    // Open agent interaction
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4

                // Header
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: {
                            switch(model.status) {
                                case "Online": return theme.successColor
                                case "Offline": return theme.warningColor
                                case "Dead": return theme.errorColor
                                default: return theme.textSecondaryColor
                            }
                        }
                    }

                    Text {
                        text: model.hostname || "Unknown"
                        font.family: theme.fontFamily
                        font.pixelSize: theme.fontSizeM
                        font.bold: true
                        color: theme.textColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                // OS & Arch
                Text {
                    text: (model.os || "Unknown") + " " + (model.arch || "")
                    font.family: theme.fontMono
                    font.pixelSize: theme.fontSizeXS
                    color: theme.textSecondaryColor
                }

                // User
                Text {
                    text: model.user || "N/A"
                    font.family: theme.fontFamily
                    font.pixelSize: theme.fontSizeS
                    color: theme.textColor
                }

                // IP
                Text {
                    text: model.ip || "N/A"
                    font.family: theme.fontMono
                    font.pixelSize: theme.fontSizeXS
                    color: theme.textSecondaryColor
                }
            }

            // Context Menu
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true

                onClicked: {
                    agentContextMenu.agentId = model.agentId
                    agentContextMenu.popup()
                }
            }

            Menu {
                id: agentContextMenu
                property string agentId

                MenuItem { text: "Interact"; onTriggered: Backend.interactAgent(agentContextMenu.agentId) }
                MenuItem { text: "File Browser"; onTriggered: Backend.openFileBrowser(agentContextMenu.agentId) }
                MenuItem { text: "Process Browser"; onTriggered: Backend.openProcessBrowser(agentContextMenu.agentId) }
                MenuItem { text: "Terminal"; onTriggered: Backend.openTerminal(agentContextMenu.agentId) }
                MenuSeparator {}
                MenuItem { text: "Screenshot"; onTriggered: Backend.takeScreenshot(agentContextMenu.agentId) }
                MenuItem { text: "Keylog Start" }
                MenuSeparator {}
                MenuItem {
                    text: "Kill Agent"
                    onTriggered: Backend.killAgent(agentContextMenu.agentId)
                }
            }
        }
    }

    // Zoom Controls
    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        width: 120
        height: 36
        radius: theme.radiusS
        color: theme.surfaceColor
        border.width: 1
        border.color: theme.borderColor

        RowLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 4

            Rectangle {
                width: 28
                height: 28
                radius: 4
                color: zoomOutMouse.containsMouse ? theme.primaryColor + "20" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "-"
                    font.family: theme.fontMono
                    font.pixelSize: 18
                    color: theme.textColor
                }

                MouseArea {
                    id: zoomOutMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: graphScale.scale -= 0.1
                }
            }

            Text {
                text: Math.round(graphScale.scale * 100) + "%"
                font.family: theme.fontMono
                font.pixelSize: theme.fontSizeS
                color: theme.textColor
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Rectangle {
                width: 28
                height: 28
                radius: 4
                color: zoomInMouse.containsMouse ? theme.primaryColor + "20" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.family: theme.fontMono
                    font.pixelSize: 18
                    color: theme.textColor
                }

                MouseArea {
                    id: zoomInMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: graphScale.scale += 0.1
                }
            }
        }
    }

    transform: Scale {
        id: graphScale
        origin.x: width / 2
        origin.y: height / 2
        property real scale: 1.0
        xScale: scale
        yScale: scale
    }

    function layoutGraph() {
        // Auto-layout algorithm for graph nodes
        // Tree layout or force-directed layout
    }

    function updateConnections() {
        connectionCanvas.requestPaint()
    }
}
