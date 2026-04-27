import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: qsTr("Add Credential")
    modal: true
    standardButtons: Dialog.NoButton
    width: 480
    height: 500

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
    property string username: ""
    property string password: ""
    property string realm: ""
    property string credType: "Password"
    property string tag: ""
    property string storage: "Memory"
    property string host: ""
    property bool editMode: false
    property string credId: ""

    // Signals
    signal saveRequested(var credData)

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
                text: editMode ? qsTr("Edit Credential") : qsTr("Add Credential")
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
            spacing: 20

            // Username
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Username")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: usernameField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Enter username")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: username
                    onTextChanged: username = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: usernameField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Password
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Password / Secret")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: passwordField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Enter password or hash")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: password
                    onTextChanged: password = text
                    echoMode: showPasswordCheck.checked ? TextInput.Normal : TextInput.Password

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: passwordField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }

                CheckBox {
                    id: showPasswordCheck
                    text: qsTr("Show password")

                    indicator: Rectangle {
                        implicitWidth: 18
                        implicitHeight: 18
                        radius: 4
                        color: parent.checked ? primaryColor : "transparent"
                        border.color: parent.checked ? primaryColor : borderColor
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            color: "white"
                            font.pixelSize: 12
                            visible: parent.parent.checked
                        }
                    }

                    contentItem: Text {
                        text: parent.text
                        font.family: "Fira Sans"
                        font.pixelSize: 12
                        color: textSecondaryColor
                        leftPadding: parent.indicator.width + 8
                    }
                }
            }

            // Realm
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Realm / Domain")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: realmField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Optional realm or domain")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: realm
                    onTextChanged: realm = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: realmField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Type and Storage
            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: qsTr("Type")
                        font.family: "Fira Sans"
                        font.pixelSize: 13
                        color: textSecondaryColor
                    }

                    ComboBox {
                        id: typeCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        model: ["Password", "Hash", "Ticket"]
                        currentIndex: model.indexOf(credType)
                        onCurrentTextChanged: credType = currentText

                        background: Rectangle {
                            color: surfaceColor
                            radius: 6
                            border.color: typeCombo.popup.visible ? primaryColor : borderColor
                            border.width: 1
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: qsTr("Storage")
                        font.family: "Fira Sans"
                        font.pixelSize: 13
                        color: textSecondaryColor
                    }

                    ComboBox {
                        id: storageCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        model: ["Memory", "Database"]
                        currentIndex: model.indexOf(storage)
                        onCurrentTextChanged: storage = currentText

                        background: Rectangle {
                            color: surfaceColor
                            radius: 6
                            border.color: storageCombo.popup.visible ? primaryColor : borderColor
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

            // Host
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Host")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: hostField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    placeholderText: qsTr("Associated host or IP")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: host
                    onTextChanged: host = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: hostField.activeFocus ? primaryColor : borderColor
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

            Text {
                visible: errorMessage !== ""
                text: errorMessage
                font.family: "Fira Sans"
                font.pixelSize: 12
                color: dangerColor
            }

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

    property string errorMessage: ""

    function isFormValid() {
        if (username.trim() === "") return false
        if (password.trim() === "") return false
        return true
    }

    function collectData() {
        return {
            credId: credId,
            username: username,
            password: password,
            realm: realm,
            type: credType,
            tag: tag,
            storage: storage,
            host: host
        }
    }

    // Public API
    function setEditMode(data) {
        editMode = true
        credId = data.credId || ""
        username = data.username || ""
        password = data.password || ""
        realm = data.realm || ""
        credType = data.type || "Password"
        tag = data.tag || ""
        storage = data.storage || "Memory"
        host = data.host || ""
    }

    function reset() {
        editMode = false
        credId = ""
        username = ""
        password = ""
        realm = ""
        credType = "Password"
        tag = ""
        storage = "Memory"
        host = ""
        errorMessage = ""
    }
}
