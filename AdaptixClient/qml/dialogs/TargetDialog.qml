import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: qsTr("Edit Target")
    modal: true
    standardButtons: Dialog.NoButton
    width: 480
    height: 520

    // Design System Colors
    readonly property color primaryColor: "#0369A1"
    readonly property color secondaryColor: "#0EA5E9"
    readonly property color ctaColor: "#22C55E"
    readonly property color backgroundColor: "#F0F9FF"
    readonly property color surfaceColor: "#FFFFFF"
    readonly property color textColor: "#0C4A6E"
    readonly property color textSecondaryColor: "#64748B"
    readonly property color borderColor: "#E2E8F0"
    readonly property color dangerColor: "#EF4444"

    // Form Properties
    property string computer: ""
    property string domain: ""
    property string address: ""
    property bool alive: true
    property string osType: "Windows"
    property string osDesc: ""
    property string tag: ""
    property string info: ""
    property bool editMode: false
    property string targetId: ""

    // Signals
    signal saveRequested(var targetData)

    // Animation
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 150 }
    }

    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 100 }
    }

    background: Rectangle {
        color: surfaceColor
        radius: 12
        border.color: borderColor
        border.width: 1
    }

    header: Rectangle {
        height: 60
        color: surfaceColor
        radius: 12

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 12
            color: parent.color
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24

            Text {
                text: editMode ? qsTr("Edit Target") : qsTr("Add Target")
                font.family: "Fira Sans"
                font.pixelSize: 20
                font.weight: Font.Bold
                color: textColor
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 32
                height: 32
                radius: 16
                color: closeMouseArea.containsMouse ? Qt.rgba(dangerColor.r, dangerColor.g, dangerColor.b, 0.1) : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "X"
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    color: textSecondaryColor
                }

                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.reject()
                }
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: borderColor
        }
    }

    contentItem: Rectangle {
        color: backgroundColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16

            // Computer Name
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Computer Name")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: computerField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Hostname or computer name")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: computer
                    onTextChanged: computer = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: computerField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Domain
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Domain")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: domainField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Domain or workgroup")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: domain
                    onTextChanged: domain = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: domainField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Address
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("IP Address")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: addressField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("IP address (e.g., 192.168.1.100)")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: address
                    onTextChanged: address = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: addressField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Alive Status
            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                CheckBox {
                    id: aliveCheck
                    text: qsTr("Target is Alive")
                    checked: alive
                    onCheckedChanged: alive = checked

                    indicator: Rectangle {
                        implicitWidth: 20
                        implicitHeight: 20
                        radius: 4
                        color: parent.checked ? ctaColor : "transparent"
                        border.color: parent.checked ? ctaColor : borderColor
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            color: "white"
                            font.pixelSize: 14
                            visible: parent.parent.checked
                        }
                    }

                    contentItem: Text {
                        text: parent.text
                        font.family: "Fira Sans"
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        color: textColor
                        leftPadding: parent.indicator.width + 10
                    }
                }
            }

            // OS Type and Description
            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: qsTr("OS Type")
                        font.family: "Fira Sans"
                        font.pixelSize: 13
                        color: textSecondaryColor
                    }

                    ComboBox {
                        id: osTypeCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        model: ["Windows", "Linux", "macOS", "Unknown"]
                        currentIndex: model.indexOf(osType)
                        onCurrentTextChanged: osType = currentText

                        background: Rectangle {
                            color: surfaceColor
                            radius: 6
                            border.color: osTypeCombo.popup.visible ? primaryColor : borderColor
                            border.width: 1
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: qsTr("OS Description")
                        font.family: "Fira Sans"
                        font.pixelSize: 13
                        color: textSecondaryColor
                    }

                    TextField {
                        id: osDescField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        placeholderText: qsTr("e.g., Windows 10 Pro")
                        font.family: "Fira Code"
                        font.pixelSize: 13
                        color: textColor
                        text: osDesc
                        onTextChanged: osDesc = text

                        background: Rectangle {
                            color: surfaceColor
                            radius: 6
                            border.color: osDescField.activeFocus ? primaryColor : borderColor
                            border.width: 1
                        }
                    }
                }
            }

            // Tag
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Tag")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: tagField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Optional tag for organization")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: tag
                    onTextChanged: tag = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: tagField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Info
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Additional Info")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: infoField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Notes or additional information")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: info
                    onTextChanged: info = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: infoField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    footer: Rectangle {
        height: 72
        color: surfaceColor

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 12
            color: parent.color
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: borderColor
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Cancel")
                font.family: "Fira Sans"
                onClicked: root.reject()

                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    color: parent.down ? Qt.darker(surfaceColor, 1.1) : surfaceColor
                    radius: 8
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
                text: editMode ? qsTr("Update") : qsTr("Add")
                font.family: "Fira Sans"
                enabled: isFormValid()
                onClicked: {
                    var data = collectData()
                    saveRequested(data)
                    root.accept()
                }

                background: Rectangle {
                    implicitWidth: 120
                    implicitHeight: 40
                    color: !parent.enabled ? Qt.rgba(ctaColor.r, ctaColor.g, ctaColor.b, 0.5)
                           : parent.down ? Qt.darker(ctaColor, 1.1) : ctaColor
                    radius: 8
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
    }

    function isFormValid() {
        return computer.trim() !== "" || address.trim() !== ""
    }

    function collectData() {
        return {
            targetId: targetId,
            computer: computer,
            domain: domain,
            address: address,
            alive: alive,
            osType: osType,
            osDesc: osDesc,
            tag: tag,
            info: info
        }
    }

    // Public API
    function setEditMode(data) {
        editMode = true
        targetId = data.targetId || ""
        computer = data.computer || ""
        domain = data.domain || ""
        address = data.address || ""
        alive = data.alive !== false
        osType = data.osType || "Windows"
        osDesc = data.osDesc || ""
        tag = data.tag || ""
        info = data.info || ""
    }

    function reset() {
        editMode = false
        targetId = ""
        computer = ""
        domain = ""
        address = ""
        alive = true
        osType = "Windows"
        osDesc = ""
        tag = ""
        info = ""
    }
}
