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
    signal createListener()
    signal editListener(string listenerName)
    signal removeListener(string listenerName)
    signal startListener(string listenerName)
    signal stopListener(string listenerName)
    signal generateAgent(string listenerName)

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
                        text: qsTr("Listeners")
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
                        placeholderText: qsTr("Search listeners...")
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

                    // Action Buttons
                    Button {
                        text: qsTr("Create Listener")
                        font.family: "Fira Sans"
                        onClicked: root.createListener()

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

            // Table
            TableView {
                id: listenersTable
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: listenersModel

                delegate: Rectangle {
                    implicitWidth: columnWidths[index] || 120
                    implicitHeight: 40
                    color: row % 2 === 0 ? surfaceColor : backgroundColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        // Status indicator for first column
                        Rectangle {
                            visible: index === 8 // Status column
                            width: 8
                            height: 8
                            radius: 4
                            color: display === "Listen" ? successColor : dangerColor
                        }

                        Text {
                            Layout.fillWidth: true
                            text: display || ""
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: index === 8 ? (display === "Listen" ? successColor : dangerColor) : textColor
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
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
                            listenersTable.selectionModel.select(currentIndex, ItemSelectionModel.Select | ItemSelectionModel.Current)
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.popup()
                            }
                        }
                    }
                }

                property var columnWidths: [150, 140, 100, 100, 120, 100, 200, 160, 100]

                // Header
                Row {
                    id: headerRow
                    x: listenersTable.contentX
                    y: 0
                    z: 2
                    spacing: 0

                    Repeater {
                        model: ["Name", "Reg Name", "Type", "Protocol", "Bind Host", "Bind Port", "C2 Hosts", "Date", "Status"]

                        Rectangle {
                            width: listenersTable.columnWidths[index] || 120
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
        }
    }

    // Context Menu
    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Start")
            onTriggered: {
                var index = listenersTable.currentIndex
                if (index.valid) {
                    var name = listenersModel.data(listenersModel.index(index.row, 0), Qt.DisplayRole)
                    root.startListener(name)
                }
            }
        }

        MenuItem {
            text: qsTr("Stop")
            onTriggered: {
                var index = listenersTable.currentIndex
                if (index.valid) {
                    var name = listenersModel.data(listenersModel.index(index.row, 0), Qt.DisplayRole)
                    root.stopListener(name)
                }
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Edit...")
            onTriggered: {
                var index = listenersTable.currentIndex
                if (index.valid) {
                    var name = listenersModel.data(listenersModel.index(index.row, 0), Qt.DisplayRole)
                    root.editListener(name)
                }
            }
        }

        MenuItem {
            text: qsTr("Generate Agent...")
            onTriggered: {
                var index = listenersTable.currentIndex
                if (index.valid) {
                    var name = listenersModel.data(listenersModel.index(index.row, 0), Qt.DisplayRole)
                    root.generateAgent(name)
                }
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Remove")
            onTriggered: {
                var index = listenersTable.currentIndex
                if (index.valid) {
                    var name = listenersModel.data(listenersModel.index(index.row, 0), Qt.DisplayRole)
                    root.removeListener(name)
                }
            }
        }
    }

    // Listener Model
    ListModel {
        id: listenersModel
    }

    // Public API
    function addListener(listenerData) {
        listenersModel.append(listenerData)
    }

    function removeListenerByName(name) {
        for (var i = 0; i < listenersModel.count; i++) {
            if (listenersModel.get(i).name === name) {
                listenersModel.remove(i)
                break
            }
        }
    }

    function updateListener(name, listenerData) {
        for (var i = 0; i < listenersModel.count; i++) {
            if (listenersModel.get(i).name === name) {
                listenersModel.set(i, listenerData)
                break
            }
        }
    }

    function clearListeners() {
        listenersModel.clear()
    }
}
