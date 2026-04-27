import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: qsTr("Listener Configuration")
    modal: true
    standardButtons: Dialog.NoButton
    width: 640
    height: 580

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
    property string listenerName: ""
    property string listenerType: ""
    property string profileName: ""
    property bool editMode: false

    // Signals
    signal createRequested(var config)
    signal saveProfile(string name)

    // Animation
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 150 }
        NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 150 }
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
                text: editMode ? qsTr("Edit Listener") : qsTr("Create Listener")
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

            // Basic Settings
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Basic Settings")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 160
                    color: surfaceColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 12

                        Text {
                            text: qsTr("Listener Name:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                            Layout.preferredWidth: 120
                        }

                        TextField {
                            id: nameField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: qsTr("Enter listener name")
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            text: listenerName
                            onTextChanged: listenerName = text

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: nameField.activeFocus ? primaryColor : borderColor
                                border.width: 1
                            }
                        }

                        Text {
                            text: qsTr("Listener Type:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                        }

                        ComboBox {
                            id: typeCombo
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            model: ["HTTP", "HTTPS", "WebSocket", "DNS", "SMB"]
                            currentIndex: 0
                            onCurrentTextChanged: {
                                listenerType = currentText
                                updateConfigPanel()
                            }

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: typeCombo.popup.visible ? primaryColor : borderColor
                                border.width: 1
                            }
                        }

                        Text {
                            text: qsTr("Bind Host:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                        }

                        TextField {
                            id: bindHostField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: "0.0.0.0"
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            text: bindHost
                            onTextChanged: bindHost = text

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: bindHostField.activeFocus ? primaryColor : borderColor
                                border.width: 1
                            }
                        }

                        Text {
                            text: qsTr("Bind Port:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                        }

                        SpinBox {
                            id: bindPortSpinBox
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            from: 1
                            to: 65535
                            value: 443

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: borderColor
                                border.width: 1
                            }
                        }
                    }
                }
            }

            // Type-specific Configuration
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                Text {
                    text: qsTr("Configuration")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: surfaceColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1
                    clip: true

                    Flickable {
                        anchors.fill: parent
                        anchors.margins: 16
                        contentWidth: width
                        contentHeight: configColumn.height
                        clip: true

                        ColumnLayout {
                            id: configColumn
                            width: parent.width
                            spacing: 16

                            // HTTP/HTTPS Settings
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: listenerType === "HTTP" || listenerType === "HTTPS"
                                spacing: 12

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("C2 Host:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 100
                                    }

                                    TextField {
                                        id: c2HostField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        placeholderText: qsTr("External IP or domain")
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: c2HostField.activeFocus ? primaryColor : borderColor
                                            border.width: 1
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Headers:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 100
                                    }

                                    TextField {
                                        id: headersField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        placeholderText: qsTr("Custom headers (optional)")
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: headersField.activeFocus ? primaryColor : borderColor
                                            border.width: 1
                                        }
                                    }
                                }

                                CheckBox {
                                    text: qsTr("Enable CORS")
                                    checked: corsEnabled
                                    onCheckedChanged: corsEnabled = checked

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
                                        font.pixelSize: 13
                                        color: textColor
                                        leftPadding: parent.indicator.width + 8
                                    }
                                }
                            }

                            // WebSocket Settings
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: listenerType === "WebSocket"
                                spacing: 12

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Endpoint:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 100
                                    }

                                    TextField {
                                        id: endpointField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        placeholderText: "/ws"
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: endpointField.activeFocus ? primaryColor : borderColor
                                            border.width: 1
                                        }
                                    }
                                }
                            }

                            // DNS Settings
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: listenerType === "DNS"
                                spacing: 12

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Domain:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 100
                                    }

                                    TextField {
                                        id: domainField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        placeholderText: qsTr("DNS domain")
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: domainField.activeFocus ? primaryColor : borderColor
                                            border.width: 1
                                        }
                                    }
                                }
                            }

                            // SSL/TLS Settings
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: listenerType === "HTTPS"
                                spacing: 12

                                Text {
                                    text: qsTr("SSL/TLS Configuration")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    font.weight: Font.Medium
                                    color: textColor
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Cert File:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 100
                                    }

                                    TextField {
                                        id: certFileField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        placeholderText: qsTr("Certificate path...")
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: certFileField.activeFocus ? primaryColor : borderColor
                                            border.width: 1
                                        }
                                    }

                                    Button {
                                        text: qsTr("Browse")
                                        font.family: "Fira Sans"

                                        background: Rectangle {
                                            implicitWidth: 70
                                            implicitHeight: 36
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

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Key File:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 100
                                    }

                                    TextField {
                                        id: keyFileField
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        placeholderText: qsTr("Key path...")
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: keyFileField.activeFocus ? primaryColor : borderColor
                                            border.width: 1
                                        }
                                    }

                                    Button {
                                        text: qsTr("Browse")
                                        font.family: "Fira Sans"

                                        background: Rectangle {
                                            implicitWidth: 70
                                            implicitHeight: 36
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

                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }

            // Profile Management
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                TextField {
                    id: profileNameField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: qsTr("Profile name (optional)...")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: profileName
                    onTextChanged: profileName = text

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: profileNameField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }

                Button {
                    text: qsTr("Load")
                    font.family: "Fira Sans"

                    background: Rectangle {
                        implicitWidth: 70
                        implicitHeight: 36
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

                Button {
                    text: qsTr("Save")
                    font.family: "Fira Sans"
                    enabled: profileName.trim() !== ""

                    background: Rectangle {
                        implicitWidth: 70
                        implicitHeight: 36
                        color: !parent.enabled ? Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.5)
                               : parent.down ? Qt.darker(primaryColor, 1.1) : primaryColor
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
            }
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
                text: editMode ? qsTr("Update") : qsTr("Create")
                font.family: "Fira Sans"
                enabled: isFormValid()
                onClicked: {
                    var config = collectConfig()
                    root.createRequested(config)
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

    // Internal Properties
    property string bindHost: "0.0.0.0"
    property bool corsEnabled: false
    property string errorMessage: ""

    function isFormValid() {
        if (listenerName.trim() === "") return false
        return true
    }

    function updateConfigPanel() {
        // Update config panel based on listener type
    }

    function collectConfig() {
        return {
            name: listenerName,
            type: listenerType,
            bindHost: bindHost,
            bindPort: bindPortSpinBox.value,
            c2Host: c2HostField.text,
            cors: corsEnabled
        }
    }

    // Public API
    function setEditMode(data) {
        editMode = true
        listenerName = data.name
        listenerType = data.type
        bindHost = data.bindHost
        bindPortSpinBox.value = data.bindPort
    }

    function resetForm() {
        editMode = false
        listenerName = ""
        listenerType = "HTTP"
        bindHost = "0.0.0.0"
        bindPortSpinBox.value = 443
        profileName = ""
        errorMessage = ""
    }
}
