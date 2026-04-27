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
    signal createTarget()
    signal editTarget(string targetId)
    signal removeTarget(string targetId)
    signal setTag(string targetId)
    signal exportTargets()

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
                        text: qsTr("Targets")
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
                        placeholderText: qsTr("Search targets...")
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
                        text: qsTr("Export")
                        font.family: "Fira Sans"
                        onClicked: root.exportTargets()

                        background: Rectangle {
                            implicitWidth: 90
                            implicitHeight: 36
                            color: parent.down ? Qt.darker(surfaceColor, 1.1) : surfaceColor
                            radius: 6
                            border.color: borderColor
                            border.width: 1
                        }

                        contentItem: Text {
                            text: parent.text
                            font.family: "Fira Sans"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: textColor
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Button {
                        text: qsTr("Add Target")
                        font.family: "Fira Sans"
                        onClicked: root.createTarget()

                        background: Rectangle {
                            implicitWidth: 120
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
                id: targetsTable
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: targetsModel

                delegate: Rectangle {
                    implicitWidth: columnWidths[index] || 100
                    implicitHeight: 40
                    color: row % 2 === 0 ? surfaceColor : backgroundColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        // Alive indicator for first column
                        Rectangle {
                            visible: index === 0
                            width: 8
                            height: 8
                            radius: 4
                            color: model.alive ? successColor : dangerColor
                        }

                        Text {
                            Layout.fillWidth: true
                            text: display || ""
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
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
                            targetsTable.selectionModel.select(currentIndex, ItemSelectionModel.Select | ItemSelectionModel.Current)
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.popup()
                            }
                        }
                    }
                }

                property var columnWidths: [80, 150, 150, 140, 100, 100, 140, 200]

                // Header
                Row {
                    id: headerRow
                    x: targetsTable.contentX
                    y: 0
                    z: 2
                    spacing: 0

                    Repeater {
                        model: ["ID", "Computer", "Domain", "Address", "Tag", "OS", "Date", "Info"]

                        Rectangle {
                            width: targetsTable.columnWidths[index] || 100
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
            text: qsTr("Edit...")
            onTriggered: {
                var index = targetsTable.currentIndex
                if (index.valid) {
                    var targetId = targetsModel.data(targetsModel.index(index.row, 0), Qt.DisplayRole)
                    root.editTarget(targetId)
                }
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Set Tag...")
            onTriggered: {
                var index = targetsTable.currentIndex
                if (index.valid) {
                    var targetId = targetsModel.data(targetsModel.index(index.row, 0), Qt.DisplayRole)
                    root.setTag(targetId)
                }
            }
        }

        MenuItem {
            text: qsTr("Copy to Clipboard")
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Remove")
            onTriggered: {
                var index = targetsTable.currentIndex
                if (index.valid) {
                    var targetId = targetsModel.data(targetsModel.index(index.row, 0), Qt.DisplayRole)
                    root.removeTarget(targetId)
                }
            }
        }
    }

    // Targets Model
    ListModel {
        id: targetsModel
    }

    // Public API
    function addTarget(targetData) {
        targetsModel.append(targetData)
    }

    function removeTargetById(targetId) {
        for (var i = 0; i < targetsModel.count; i++) {
            if (targetsModel.get(i).targetId === targetId) {
                targetsModel.remove(i)
                break
            }
        }
    }

    function updateTarget(targetId, targetData) {
        for (var i = 0; i < targetsModel.count; i++) {
            if (targetsModel.get(i).targetId === targetId) {
                targetsModel.set(i, targetData)
                break
            }
        }
    }

    function clearTargets() {
        targetsModel.clear()
    }
}
