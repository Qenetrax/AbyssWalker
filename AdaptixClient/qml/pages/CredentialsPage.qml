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
    property string typeFilter: ""
    property string storageFilter: ""

    // Signals
    signal createCredential()
    signal editCredential(string credId)
    signal removeCredential(string credId)
    signal exportCredentials()
    signal copyToClipboard(string credId)

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
                        text: qsTr("Credentials")
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
                        Layout.preferredWidth: 240
                        Layout.preferredHeight: 36
                        placeholderText: qsTr("Search credentials...")
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

                    // Type Filter
                    ComboBox {
                        id: typeCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 36
                        model: ["All Types", "Password", "Hash", "Ticket"]
                        currentIndex: 0
                        displayText: currentText
                        onCurrentTextChanged: {
                            typeFilter = currentIndex === 0 ? "" : currentText
                        }

                        background: Rectangle {
                            color: backgroundColor
                            radius: 6
                            border.color: typeCombo.popup.visible ? primaryColor : borderColor
                            border.width: 1
                        }
                    }

                    // Storage Filter
                    ComboBox {
                        id: storageCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 36
                        model: ["All Storage", "Memory", "Database"]
                        currentIndex: 0
                        displayText: currentText
                        onCurrentTextChanged: {
                            storageFilter = currentIndex === 0 ? "" : currentText
                        }

                        background: Rectangle {
                            color: backgroundColor
                            radius: 6
                            border.color: storageCombo.popup.visible ? primaryColor : borderColor
                            border.width: 1
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Action Buttons
                    Button {
                        text: qsTr("Export")
                        font.family: "Fira Sans"
                        onClicked: root.exportCredentials()

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
                        text: qsTr("Add Credential")
                        font.family: "Fira Sans"
                        onClicked: root.createCredential()

                        background: Rectangle {
                            implicitWidth: 130
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
                id: credsTable
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: credsModel

                delegate: Rectangle {
                    implicitWidth: columnWidths[index] || 100
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
                            credsTable.selectionModel.select(currentIndex, ItemSelectionModel.Select | ItemSelectionModel.Current)
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.popup()
                            }
                        }
                    }
                }

                property var columnWidths: [80, 120, 150, 100, 80, 100, 140, 100, 100, 120]

                // Header
                Row {
                    id: headerRow
                    x: credsTable.contentX
                    y: 0
                    z: 2
                    spacing: 0

                    Repeater {
                        model: ["ID", "Username", "Password", "Realm", "Type", "Tag", "Date", "Storage", "Agent", "Host"]

                        Rectangle {
                            width: credsTable.columnWidths[index] || 100
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
            text: qsTr("Copy to Clipboard")
            onTriggered: {
                var index = credsTable.currentIndex
                if (index.valid) {
                    var credId = credsModel.data(credsModel.index(index.row, 0), Qt.DisplayRole)
                    root.copyToClipboard(credId)
                }
            }
        }

        MenuItem {
            text: qsTr("Edit...")
            onTriggered: {
                var index = credsTable.currentIndex
                if (index.valid) {
                    var credId = credsModel.data(credsModel.index(index.row, 0), Qt.DisplayRole)
                    root.editCredential(credId)
                }
            }
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Set Tag...")
        }

        MenuItem {
            text: qsTr("Remove")
            onTriggered: {
                var index = credsTable.currentIndex
                if (index.valid) {
                    var credId = credsModel.data(credsModel.index(index.row, 0), Qt.DisplayRole)
                    root.removeCredential(credId)
                }
            }
        }
    }

    // Credentials Model
    ListModel {
        id: credsModel
    }

    // Public API
    function addCredential(credData) {
        credsModel.append(credData)
    }

    function removeCredentialById(credId) {
        for (var i = 0; i < credsModel.count; i++) {
            if (credsModel.get(i).credId === credId) {
                credsModel.remove(i)
                break
            }
        }
    }

    function updateCredential(credId, credData) {
        for (var i = 0; i < credsModel.count; i++) {
            if (credsModel.get(i).credId === credId) {
                credsModel.set(i, credData)
                break
            }
        }
    }

    function clearCredentials() {
        credsModel.clear()
    }
}
