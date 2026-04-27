import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: qsTr("Create Tunnel")
    modal: true
    standardButtons: Dialog.NoButton
    width: 520
    height: 480

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
    property string tunnelType: "SOCKS5"
    property string agentId: ""
    property string description: ""

    // Signals
    signal createRequested(var config)

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
                text: qsTr("Create Tunnel")
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

            // Tunnel Type Selection
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Tunnel Type")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                RowLayout {
                    spacing: 12

                    Repeater {
                        model: [
                            { name: "SOCKS5", enabled: socks5Enabled },
                            { name: "SOCKS4", enabled: socks4Enabled },
                            { name: "LPF", enabled: lpfEnabled },
                            { name: "RPF", enabled: rpfEnabled }
                        ]

                        Rectangle {
                            width: typeText.width + 24
                            height: 40
                            radius: 8
                            color: !modelData.enabled ? Qt.rgba(0.9, 0.9, 0.9, 1)
                                   : tunnelType === modelData.name ? primaryColor : "transparent"
                            border.color: !modelData.enabled ? borderColor
                                          : tunnelType === modelData.name ? primaryColor : borderColor
                            border.width: 1
                            opacity: modelData.enabled ? 1 : 0.5

                            Text {
                                id: typeText
                                anchors.centerIn: parent
                                text: modelData.name
                                font.family: "Fira Sans"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                                color: !modelData.enabled ? textSecondaryColor
                                       : tunnelType === modelData.name ? "white" : textColor
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: modelData.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    if (modelData.enabled) {
                                        tunnelType = modelData.name
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Endpoint Selection
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Endpoint (Agent)")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    color: surfaceColor
                    radius: 8
                    border.color: endpointCombo.popup.visible ? primaryColor : borderColor
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12

                        Text {
                            text: agentId || qsTr("Select agent...")
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: agentId ? textColor : textSecondaryColor
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        Text {
                            text: "V"
                            font.family: "Fira Sans"
                            font.pixelSize: 12
                            color: textSecondaryColor
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: endpointCombo.open()
                    }

                    Popup {
                        id: endpointCombo
                        width: parent.width
                        implicitHeight: endpointContentColumn.height + 16
                        y: parent.height + 4

                        background: Rectangle {
                            color: surfaceColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1
                        }

                        Column {
                            id: endpointContentColumn
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 8
                            spacing: 4

                            Repeater {
                                model: endpointModel

                                Rectangle {
                                    width: endpointContentColumn.width - 16
                                    height: 36
                                    color: endpointMouseArea.containsMouse ? backgroundColor : "transparent"
                                    radius: 6

                                    Text {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        text: modelData
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    MouseArea {
                                        id: endpointMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            agentId = modelData
                                            endpointCombo.close()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Tunnel Configuration
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

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        // Description
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Text {
                                text: qsTr("Description:")
                                font.family: "Fira Sans"
                                font.pixelSize: 13
                                color: textSecondaryColor
                                Layout.preferredWidth: 100
                            }

                            TextField {
                                id: descField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36
                                placeholderText: qsTr("Optional description")
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                color: textColor
                                text: description
                                onTextChanged: description = text

                                background: Rectangle {
                                    color: backgroundColor
                                    radius: 6
                                    border.color: descField.activeFocus ? primaryColor : borderColor
                                    border.width: 1
                                }
                            }
                        }

                        // Bind Address
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Text {
                                text: qsTr("Bind Address:")
                                font.family: "Fira Sans"
                                font.pixelSize: 13
                                color: textSecondaryColor
                                Layout.preferredWidth: 100
                            }

                            TextField {
                                id: bindAddrField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36
                                placeholderText: "127.0.0.1"
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                color: textColor

                                background: Rectangle {
                                    color: backgroundColor
                                    radius: 6
                                    border.color: bindAddrField.activeFocus ? primaryColor : borderColor
                                    border.width: 1
                                }
                            }

                            Text {
                                text: qsTr("Port:")
                                font.family: "Fira Sans"
                                font.pixelSize: 13
                                color: textSecondaryColor
                            }

                            SpinBox {
                                id: bindPortSpinBox
                                Layout.preferredWidth: 100
                                Layout.preferredHeight: 36
                                from: 1
                                to: 65535
                                value: 1080

                                background: Rectangle {
                                    color: backgroundColor
                                    radius: 6
                                    border.color: borderColor
                                    border.width: 1
                                }
                            }
                        }

                        // SOCKS5 Authentication
                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: tunnelType === "SOCKS5"
                            spacing: 12

                            CheckBox {
                                id: authCheck
                                text: qsTr("Enable Authentication")

                                indicator: Rectangle {
                                    implicitWidth: 18
                                    implicitHeight: 18
                                    radius: 4
                                    color: authCheck.checked ? primaryColor : "transparent"
                                    border.color: authCheck.checked ? primaryColor : borderColor
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: "white"
                                        font.pixelSize: 12
                                        visible: authCheck.checked
                                    }
                                }

                                contentItem: Text {
                                    text: authCheck.text
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                    leftPadding: authCheck.indicator.width + 8
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                visible: authCheck.checked

                                Text {
                                    text: qsTr("Username:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textSecondaryColor
                                    Layout.preferredWidth: 80
                                }

                                TextField {
                                    id: authUserField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    placeholderText: qsTr("Username")
                                    font.family: "Fira Code"
                                    font.pixelSize: 13
                                    color: textColor

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 6
                                        border.color: authUserField.activeFocus ? primaryColor : borderColor
                                        border.width: 1
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                visible: authCheck.checked

                                Text {
                                    text: qsTr("Password:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textSecondaryColor
                                    Layout.preferredWidth: 80
                                }

                                TextField {
                                    id: authPassField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    placeholderText: qsTr("Password")
                                    font.family: "Fira Code"
                                    font.pixelSize: 13
                                    color: textColor
                                    echoMode: TextInput.Password

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 6
                                        border.color: authPassField.activeFocus ? primaryColor : borderColor
                                        border.width: 1
                                    }
                                }
                            }
                        }

                        // LPF/RPF Target Configuration
                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: tunnelType === "LPF" || tunnelType === "RPF"
                            spacing: 12

                            Text {
                                text: qsTr("Target Configuration")
                                font.family: "Fira Sans"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                                color: textColor
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Text {
                                    text: qsTr("Target Address:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textSecondaryColor
                                    Layout.preferredWidth: 100
                                }

                                TextField {
                                    id: targetAddrField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    placeholderText: qsTr("Target IP or hostname")
                                    font.family: "Fira Code"
                                    font.pixelSize: 13
                                    color: textColor

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 6
                                        border.color: targetAddrField.activeFocus ? primaryColor : borderColor
                                        border.width: 1
                                    }
                                }

                                Text {
                                    text: qsTr("Port:")
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textSecondaryColor
                                }

                                SpinBox {
                                    id: targetPortSpinBox
                                    Layout.preferredWidth: 100
                                    Layout.preferredHeight: 36
                                    from: 1
                                    to: 65535
                                    value: 80

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 6
                                        border.color: borderColor
                                        border.width: 1
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
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
                text: qsTr("Create")
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
    property bool socks5Enabled: true
    property bool socks4Enabled: true
    property bool lpfEnabled: true
    property bool rpfEnabled: true
    property var endpointModel: []

    function isFormValid() {
        if (agentId === "") return false
        return true
    }

    function collectConfig() {
        var config = {
            type: tunnelType,
            agentId: agentId,
            description: description,
            bindAddress: bindAddrField.text || "127.0.0.1",
            bindPort: bindPortSpinBox.value
        }

        if (tunnelType === "SOCKS5" && authCheck.checked) {
            config.auth = true
            config.username = authUserField.text
            config.password = authPassField.text
        }

        if (tunnelType === "LPF" || tunnelType === "RPF") {
            config.targetAddress = targetAddrField.text
            config.targetPort = targetPortSpinBox.value
        }

        return config
    }

    // Public API
    function setEndpoints(endpoints) {
        endpointModel = endpoints
    }

    function setTunnelTypes(socks5, socks4, lpf, rpf) {
        socks5Enabled = socks5
        socks4Enabled = socks4
        lpfEnabled = lpf
        rpfEnabled = rpf
    }

    function setAgent(agentId) {
        root.agentId = agentId
    }
}
