import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: qsTr("Agent Builder")
    modal: true
    standardButtons: Dialog.NoButton
    width: 720
    height: 640

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
    property string selectedListener: ""
    property string selectedAgent: ""
    property string profileName: ""
    property var configOptions: ({})
    property var selectedListeners: []

    // Signals
    signal buildRequested(var config)
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
                text: qsTr("Agent Builder")
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

        RowLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 24

            // Left Panel - Configuration
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                // Listener Selection
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: qsTr("Listener")
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
                        border.color: listenerCombo.popup.visible ? primaryColor : borderColor
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text {
                                text: selectedListener || qsTr("Select a listener...")
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                color: selectedListener ? textColor : textSecondaryColor
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
                            onClicked: listenerCombo.open()
                        }

                        Popup {
                            id: listenerCombo
                            width: parent.width
                            implicitHeight: contentColumn.height + 16
                            y: parent.height + 4

                            background: Rectangle {
                                color: surfaceColor
                                radius: 8
                                border.color: borderColor
                                border.width: 1
                            }

                            Column {
                                id: contentColumn
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: 8
                                spacing: 4

                                Repeater {
                                    model: listenerModel

                                    Rectangle {
                                        width: contentColumn.width - 16
                                        height: 36
                                        color: listenerMouseArea.containsMouse ? backgroundColor : "transparent"
                                        radius: 6

                                        Text {
                                            anchors.fill: parent
                                            anchors.leftMargin: 12
                                            text: modelData.name
                                            font.family: "Fira Code"
                                            font.pixelSize: 13
                                            color: textColor
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        MouseArea {
                                            id: listenerMouseArea
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                selectedListener = modelData.name
                                                listenerCombo.close()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Agent Type Selection
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: qsTr("Agent Type")
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
                        border.color: agentCombo.popup.visible ? primaryColor : borderColor
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text {
                                text: selectedAgent || qsTr("Select agent type...")
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                color: selectedAgent ? textColor : textSecondaryColor
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
                            onClicked: agentCombo.open()
                        }

                        Popup {
                            id: agentCombo
                            width: parent.width
                            implicitHeight: agentContentColumn.height + 16
                            y: parent.height + 4

                            background: Rectangle {
                                color: surfaceColor
                                radius: 8
                                border.color: borderColor
                                border.width: 1
                            }

                            Column {
                                id: agentContentColumn
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: 8
                                spacing: 4

                                Repeater {
                                    model: agentModel

                                    Rectangle {
                                        width: agentContentColumn.width - 16
                                        height: 36
                                        color: agentMouseArea.containsMouse ? backgroundColor : "transparent"
                                        radius: 6

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 12
                                            anchors.rightMargin: 12
                                            spacing: 8

                                            Text {
                                                text: modelData.name
                                                font.family: "Fira Code"
                                                font.pixelSize: 13
                                                color: textColor
                                            }

                                            Text {
                                                text: modelData.description
                                                font.family: "Fira Sans"
                                                font.pixelSize: 11
                                                color: textSecondaryColor
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                        }

                                        MouseArea {
                                            id: agentMouseArea
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                selectedAgent = modelData.name
                                                agentCombo.close()
                                                loadAgentConfig(modelData.name)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Configuration Options
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

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

                                // Platform Selection
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        text: qsTr("Platform")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: textColor
                                    }

                                    RowLayout {
                                        spacing: 12

                                        Repeater {
                                            model: ["Windows", "Linux", "macOS"]

                                            Rectangle {
                                                width: platformText.width + 24
                                                height: 32
                                                radius: 6
                                                color: selectedPlatform === modelData ? primaryColor : "transparent"
                                                border.color: selectedPlatform === modelData ? primaryColor : borderColor
                                                border.width: 1

                                                Text {
                                                    id: platformText
                                                    anchors.centerIn: parent
                                                    text: modelData
                                                    font.family: "Fira Sans"
                                                    font.pixelSize: 12
                                                    color: selectedPlatform === modelData ? "white" : textColor
                                                }

                                                MouseArea {
                                                    anchors.fill: parent
                                                    cursorShape: Qt.PointingHandCursor
                                                    onClicked: selectedPlatform = modelData
                                                }
                                            }
                                        }
                                    }
                                }

                                // Architecture Selection
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        text: qsTr("Architecture")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: textColor
                                    }

                                    RowLayout {
                                        spacing: 12

                                        Repeater {
                                            model: ["x64", "x86"]

                                            Rectangle {
                                                width: archText.width + 24
                                                height: 32
                                                radius: 6
                                                color: selectedArch === modelData ? primaryColor : "transparent"
                                                border.color: selectedArch === modelData ? primaryColor : borderColor
                                                border.width: 1

                                                Text {
                                                    id: archText
                                                    anchors.centerIn: parent
                                                    text: modelData
                                                    font.family: "Fira Sans"
                                                    font.pixelSize: 12
                                                    color: selectedArch === modelData ? "white" : textColor
                                                }

                                                MouseArea {
                                                    anchors.fill: parent
                                                    cursorShape: Qt.PointingHandCursor
                                                    onClicked: selectedArch = modelData
                                                }
                                            }
                                        }
                                    }
                                }

                                // Sleep Time
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Sleep Time:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 120
                                    }

                                    SpinBox {
                                        id: sleepSpinBox
                                        Layout.preferredWidth: 120
                                        from: 1
                                        to: 3600
                                        value: 5

                                        background: Rectangle {
                                            color: backgroundColor
                                            radius: 6
                                            border.color: borderColor
                                            border.width: 1
                                        }
                                    }

                                    Text {
                                        text: qsTr("seconds")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                    }
                                }

                                // Jitter
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Text {
                                        text: qsTr("Jitter:")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        color: textSecondaryColor
                                        Layout.preferredWidth: 120
                                    }

                                    Slider {
                                        id: jitterSlider
                                        Layout.fillWidth: true
                                        from: 0
                                        to: 100
                                        value: 0

                                        background: Rectangle {
                                            x: jitterSlider.leftPadding
                                            y: jitterSlider.topPadding + jitterSlider.availableHeight / 2 - height / 2
                                            width: jitterSlider.availableWidth
                                            height: 4
                                            radius: 2
                                            color: borderColor

                                            Rectangle {
                                                width: jitterSlider.visualPosition * parent.width
                                                height: parent.height
                                                radius: 2
                                                color: primaryColor
                                            }
                                        }

                                        handle: Rectangle {
                                            x: jitterSlider.leftPadding + jitterSlider.visualPosition * (jitterSlider.availableWidth - width)
                                            y: jitterSlider.topPadding + jitterSlider.availableHeight / 2 - height / 2
                                            width: 16
                                            height: 16
                                            radius: 8
                                            color: jitterSlider.pressed ? Qt.darker(primaryColor, 1.1) : primaryColor
                                        }
                                    }

                                    Text {
                                        text: Math.round(jitterSlider.value) + "%"
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor
                                        Layout.preferredWidth: 50
                                    }
                                }

                                // Evasion Options
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        text: qsTr("Evasion Options")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: textColor
                                    }

                                    CheckBox {
                                        text: qsTr("Enable Unhooking")
                                        checked: unhookingEnabled
                                        onCheckedChanged: unhookingEnabled = checked

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

                                    CheckBox {
                                        text: qsTr("Enable Anti-Debug")
                                        checked: antiDebugEnabled
                                        onCheckedChanged: antiDebugEnabled = checked

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

                                    CheckBox {
                                        text: qsTr("Enable Process Migration")
                                        checked: migrationEnabled
                                        onCheckedChanged: migrationEnabled = checked

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
                            }
                        }
                    }
                }
            }

            // Right Panel - Build Output
            ColumnLayout {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                spacing: 16

                Text {
                    text: qsTr("Build Output")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#1E293B"
                    radius: 8

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 12

                        TextEdit {
                            id: buildOutput
                            text: qsTr("Ready to build...")
                            font.family: "Fira Code"
                            font.pixelSize: 12
                            color: "#94A3B8"
                            selectByMouse: true
                            readOnly: true
                            wrapMode: TextEdit.Wrap
                        }
                    }
                }

                // Profile Name
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: profileNameField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        placeholderText: qsTr("Profile name...")
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
                        text: qsTr("Save")
                        font.family: "Fira Sans"
                        enabled: profileName.trim() !== ""
                        onClicked: root.saveProfile(profileName)

                        background: Rectangle {
                            implicitWidth: 60
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
                id: buildButton
                text: qsTr("Build")
                font.family: "Fira Sans"
                enabled: isFormValid()
                onClicked: {
                    var config = collectConfig()
                    root.buildRequested(config)
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
    property string selectedPlatform: "Windows"
    property string selectedArch: "x64"
    property bool unhookingEnabled: false
    property bool antiDebugEnabled: false
    property bool migrationEnabled: false

    // Models
    property var listenerModel: []
    property var agentModel: []

    function isFormValid() {
        return selectedListener !== "" && selectedAgent !== ""
    }

    function loadAgentConfig(agentName) {
        // Load agent-specific configuration
        buildOutput.text = qsTr("Loaded configuration for: ") + agentName
    }

    function collectConfig() {
        return {
            listener: selectedListener,
            agent: selectedAgent,
            platform: selectedPlatform,
            arch: selectedArch,
            sleep: sleepSpinBox.value,
            jitter: jitterSlider.value,
            unhooking: unhookingEnabled,
            antiDebug: antiDebugEnabled,
            migration: migrationEnabled
        }
    }

    function setBuildOutput(text) {
        buildOutput.text = text
    }

    function appendBuildOutput(text) {
        buildOutput.text += "\n" + text
    }

    // Public API
    function setListeners(listeners) {
        listenerModel = listeners
    }

    function setAgents(agents) {
        agentModel = agents
    }
}
