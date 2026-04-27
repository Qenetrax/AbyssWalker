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

    property int viewMode: 0 // 0 = Table, 1 = Topology
    property string filterText: ""
    property bool showOnlyActive: false

    // Signals
    signal agentSelected(string agentId)
    signal agentDoubleClicked(string agentId)
    signal generateAgent()
    signal removeAgent(string agentId)
    signal interactAgent(string agentId)

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

                    // View Mode Switch
                    RowLayout {
                        spacing: 4

                        Rectangle {
                            width: 36
                            height: 36
                            radius: 6
                            color: viewMode === 0 ? primaryColor : "transparent"
                            border.color: viewMode === 0 ? primaryColor : borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Table")
                                font.family: "Fira Sans"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: viewMode === 0 ? "white" : textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: viewMode = 0
                            }
                        }

                        Rectangle {
                            width: 36
                            height: 36
                            radius: 6
                            color: viewMode === 1 ? primaryColor : "transparent"
                            border.color: viewMode === 1 ? primaryColor : borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Topo")
                                font.family: "Fira Sans"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                color: viewMode === 1 ? "white" : textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: viewMode = 1
                            }
                        }
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
                        placeholderText: qsTr("Search agents...")
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

                    // Only Active Checkbox
                    CheckBox {
                        id: activeOnlyCheck
                        text: qsTr("Active Only")
                        checked: showOnlyActive
                        onCheckedChanged: showOnlyActive = checked

                        indicator: Rectangle {
                            implicitWidth: 18
                            implicitHeight: 18
                            radius: 4
                            color: activeOnlyCheck.checked ? primaryColor : "transparent"
                            border.color: activeOnlyCheck.checked ? primaryColor : borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                color: "white"
                                font.pixelSize: 12
                                visible: activeOnlyCheck.checked
                            }
                        }

                        contentItem: Text {
                            text: activeOnlyCheck.text
                            font.family: "Fira Sans"
                            font.pixelSize: 14
                            color: textColor
                            leftPadding: activeOnlyCheck.indicator.width + 8
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Action Buttons
                    Button {
                        text: qsTr("Generate Agent")
                        font.family: "Fira Sans"
                        icon.source: "qrc:/icons/code.svg"
                        onClicked: root.generateAgent()

                        background: Rectangle {
                            implicitWidth: 140
                            implicitHeight: 36
                            color: parent.down ? Qt.darker(ctaColor, 1.1) : ctaColor
                            radius: 6
                        }

                        contentItem: Text {
                            text: parent.text
                            font.family: "Fira Sans"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
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

            // Content Area
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // Table View
                TableView {
                    id: agentsTable
                    anchors.fill: parent
                    visible: viewMode === 0
                    clip: true
                    model: agentsModel
                    selectionModel: ItemSelectionModel {}

                    delegate: Rectangle {
                        implicitWidth: columnWidths[index] || 120
                        implicitHeight: 40
                        color: row % 2 === 0 ? surfaceColor : backgroundColor

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            text: display || ""
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 1
                            color: borderColor
                            opacity: 0.5
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onClicked: function(mouse) {
                                if (mouse.button === Qt.LeftButton) {
                                    agentsTable.selectionModel.select(currentIndex, ItemSelectionModel.Select | ItemSelectionModel.Current)
                                } else if (mouse.button === Qt.RightButton) {
                                    contextMenu.popup()
                                }
                            }
                            onDoubleClicked: {
                                var agentId = agentsModel.data(agentsModel.index(row, 0), Qt.DisplayRole)
                                root.agentDoubleClicked(agentId)
                            }
                        }
                    }

                    // Column Widths
                    property var columnWidths: [100, 80, 100, 120, 100, 100, 100, 100, 150, 120, 60, 60, 100, 140, 140, 100]

                    // Header
                    Row {
                        id: headerRow
                        x: agentsTable.contentX
                        y: 0
                        z: 2
                        spacing: 0

                        Repeater {
                            model: ["Agent ID", "Type", "External", "Listener", "Internal", "Domain", "Computer", "User", "OS", "Process", "PID", "TID", "Tags", "Created", "Last", "Sleep"]

                            Rectangle {
                                width: agentsTable.columnWidths[index] || 120
                                height: 36
                                color: surfaceColor

                                Text {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    text: modelData
                                    font.family: "Fira Sans"
                                    font.pixelSize: 12
                                    font.weight: Font.DemiBold
                                    color: textSecondaryColor
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Rectangle {
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    width: 1
                                    color: borderColor
                                }

                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    height: 1
                                    color: borderColor
                                }
                            }
                        }
                    }
                }

                // Topology View
                Item {
                    id: topologyView
                    anchors.fill: parent
                    visible: viewMode === 1
                    clip: true

                    Flickable {
                        anchors.fill: parent
                        contentWidth: topologyCanvas.width
                        contentHeight: topologyCanvas.height
                        clip: true

                        Canvas {
                            id: topologyCanvas
                            width: 2000
                            height: 1500

                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.fillStyle = backgroundColor
                                ctx.fillRect(0, 0, width, height)

                                // Draw nodes and connections
                                // This will be populated from C++ model
                            }
                        }
                    }

                    // Topology Controls
                    Column {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 16
                        spacing: 8

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 8
                            color: surfaceColor
                            border.color: borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "+"
                                font.pixelSize: 20
                                color: textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                            }
                        }

                        Rectangle {
                            width: 40
                            height: 40
                            radius: 8
                            color: surfaceColor
                            border.color: borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "-"
                                font.pixelSize: 20
                                color: textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                            }
                        }
                    }
                }
            }
        }
    }

    // Context Menu
    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Interact")
            onTriggered: {
                var index = agentsTable.currentIndex
                if (index.valid) {
                    var agentId = agentsModel.data(agentsModel.index(index.row, 0), Qt.DisplayRole)
                    root.interactAgent(agentId)
                }
            }
        }

        MenuItem {
            text: qsTr("Remove")
            onTriggered: {
                var index = agentsTable.currentIndex
                if (index.valid) {
                    var agentId = agentsModel.data(agentsModel.index(index.row, 0), Qt.DisplayRole)
                    root.removeAgent(agentId)
                }
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Mark Active")
        }

        MenuItem {
            text: qsTr("Mark Inactive")
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Set Color...")
        }

        MenuItem {
            text: qsTr("Set Tag...")
        }
    }

    // Agent Model (placeholder)
    ListModel {
        id: agentsModel
        // Will be populated from C++
    }

    // Public API
    function addAgent(agentData) {
        agentsModel.append(agentData)
    }

    function removeAgentById(agentId) {
        for (var i = 0; i < agentsModel.count; i++) {
            if (agentsModel.get(i).agentId === agentId) {
                agentsModel.remove(i)
                break
            }
        }
    }

    function updateAgent(agentId, agentData) {
        for (var i = 0; i < agentsModel.count; i++) {
            if (agentsModel.get(i).agentId === agentId) {
                agentsModel.set(i, agentData)
                break
            }
        }
    }

    function clearAgents() {
        agentsModel.clear()
    }

    function refreshTopology() {
        topologyCanvas.requestPaint()
    }
}
