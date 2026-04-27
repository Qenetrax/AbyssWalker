import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: qsTr("Settings")
    modal: true
    standardButtons: Dialog.NoButton
    width: 720
    height: 560

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

    // Settings Properties
    property string currentTheme: "Light"
    property int fontSize: 12
    property string fontFamily: "Fira Code"
    property int terminalFontSize: 12
    property bool consoleTimestamp: true
    property bool consoleAutoScroll: true
    property bool consoleNoWrap: false
    property bool consoleShowBackground: true
    property string consoleTheme: "Default"

    // Signals
    signal applyRequested(var settings)
    signal resetRequested()

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
                text: qsTr("Settings")
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
            spacing: 0

            // Sidebar
            Rectangle {
                Layout.preferredWidth: 200
                Layout.fillHeight: true
                color: surfaceColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 4

                    Repeater {
                        model: [
                            { name: qsTr("General"), icon: "G" },
                            { name: qsTr("Console"), icon: "C" },
                            { name: qsTr("Sessions"), icon: "S" },
                            { name: qsTr("Tasks"), icon: "T" },
                            { name: qsTr("Appearance"), icon: "A" }
                        ]

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            radius: 8
                            color: categoryIndex === index ? Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.1) : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 10

                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 6
                                    color: categoryIndex === index ? primaryColor : "transparent"
                                    border.color: categoryIndex === index ? primaryColor : borderColor
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.icon
                                        font.family: "Fira Sans"
                                        font.pixelSize: 11
                                        font.weight: Font.Bold
                                        color: categoryIndex === index ? "white" : textSecondaryColor
                                    }
                                }

                                Text {
                                    text: modelData.name
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    font.weight: categoryIndex === index ? Font.DemiBold : Font.Normal
                                    color: categoryIndex === index ? primaryColor : textColor
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: categoryIndex = index
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 1
                    color: borderColor
                }
            }

            // Content Area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: backgroundColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 20

                    // Header
                    Text {
                        text: getCategoryTitle(categoryIndex)
                        font.family: "Fira Sans"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: textColor
                    }

                    // Settings Content
                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: categoryIndex

                        // General Settings
                        Rectangle {
                            color: "transparent"

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 16

                                // Theme
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 70
                                    color: surfaceColor
                                    radius: 8
                                    border.color: borderColor
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 16
                                        spacing: 16

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Text {
                                                text: qsTr("Theme")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 13
                                                font.weight: Font.DemiBold
                                                color: textColor
                                            }

                                            Text {
                                                text: qsTr("Select application theme")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 12
                                                color: textSecondaryColor
                                            }
                                        }

                                        ComboBox {
                                            id: themeCombo
                                            Layout.preferredWidth: 180
                                            Layout.preferredHeight: 36
                                            model: ["Light", "Dark", "System"]
                                            currentIndex: model.indexOf(currentTheme)
                                            onCurrentTextChanged: currentTheme = currentText

                                            background: Rectangle {
                                                color: backgroundColor
                                                radius: 6
                                                border.color: themeCombo.popup.visible ? primaryColor : borderColor
                                                border.width: 1
                                            }
                                        }
                                    }
                                }

                                // Font Size
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 70
                                    color: surfaceColor
                                    radius: 8
                                    border.color: borderColor
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 16
                                        spacing: 16

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Text {
                                                text: qsTr("Font Size")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 13
                                                font.weight: Font.DemiBold
                                                color: textColor
                                            }

                                            Text {
                                                text: qsTr("Base font size for interface")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 12
                                                color: textSecondaryColor
                                            }
                                        }

                                        SpinBox {
                                            id: fontSizeSpin
                                            Layout.preferredWidth: 100
                                            Layout.preferredHeight: 36
                                            from: 8
                                            to: 24
                                            value: fontSize
                                            onValueChanged: fontSize = value

                                            background: Rectangle {
                                                color: backgroundColor
                                                radius: 6
                                                border.color: borderColor
                                                border.width: 1
                                            }
                                        }
                                    }
                                }

                                // Font Family
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 70
                                    color: surfaceColor
                                    radius: 8
                                    border.color: borderColor
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 16
                                        spacing: 16

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Text {
                                                text: qsTr("Font Family")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 13
                                                font.weight: Font.DemiBold
                                                color: textColor
                                            }

                                            Text {
                                                text: qsTr("Primary font family")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 12
                                                color: textSecondaryColor
                                            }
                                        }

                                        ComboBox {
                                            id: fontFamilyCombo
                                            Layout.preferredWidth: 180
                                            Layout.preferredHeight: 36
                                            model: ["Fira Code", "Fira Sans", "Consolas", "Source Code Pro"]
                                            currentIndex: model.indexOf(fontFamily)
                                            onCurrentTextChanged: fontFamily = currentText

                                            background: Rectangle {
                                                color: backgroundColor
                                                radius: 6
                                                border.color: fontFamilyCombo.popup.visible ? primaryColor : borderColor
                                                border.width: 1
                                            }
                                        }
                                    }
                                }

                                Item { Layout.fillHeight: true }
                            }
                        }

                        // Console Settings
                        Rectangle {
                            color: "transparent"

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 16

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
                                        spacing: 12

                                        Text {
                                            text: qsTr("Display Options")
                                            font.family: "Fira Sans"
                                            font.pixelSize: 14
                                            font.weight: Font.DemiBold
                                            color: textColor
                                        }

                                        CheckBox {
                                            text: qsTr("Show Timestamps")
                                            checked: consoleTimestamp
                                            onCheckedChanged: consoleTimestamp = checked

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
                                            text: qsTr("Auto-scroll to Bottom")
                                            checked: consoleAutoScroll
                                            onCheckedChanged: consoleAutoScroll = checked

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
                                            text: qsTr("No Wrap")
                                            checked: consoleNoWrap
                                            onCheckedChanged: consoleNoWrap = checked

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
                                            text: qsTr("Show Background Image")
                                            checked: consoleShowBackground
                                            onCheckedChanged: consoleShowBackground = checked

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

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 12

                                            Text {
                                                text: qsTr("Console Theme:")
                                                font.family: "Fira Sans"
                                                font.pixelSize: 13
                                                color: textSecondaryColor
                                            }

                                            ComboBox {
                                                id: consoleThemeCombo
                                                Layout.preferredWidth: 180
                                                Layout.preferredHeight: 36
                                                model: ["Default", "Dark", "Light", "Solarized Dark", "Solarized Light"]
                                                currentIndex: model.indexOf(consoleTheme)
                                                onCurrentTextChanged: consoleTheme = currentText

                                                background: Rectangle {
                                                    color: backgroundColor
                                                    radius: 6
                                                    border.color: consoleThemeCombo.popup.visible ? primaryColor : borderColor
                                                    border.width: 1
                                                }
                                            }
                                        }

                                        Item { Layout.fillHeight: true }
                                    }
                                }
                            }
                        }

                        // Sessions Settings
                        Rectangle {
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Session column visibility settings")
                                font.family: "Fira Sans"
                                font.pixelSize: 14
                                color: textSecondaryColor
                            }
                        }

                        // Tasks Settings
                        Rectangle {
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Task column visibility settings")
                                font.family: "Fira Sans"
                                font.pixelSize: 14
                                color: textSecondaryColor
                            }
                        }

                        // Appearance Settings
                        Rectangle {
                            color: "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Tab blink and animation settings")
                                font.family: "Fira Sans"
                                font.pixelSize: 14
                                color: textSecondaryColor
                            }
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

            Button {
                text: qsTr("Reset to Defaults")
                font.family: "Fira Sans"
                onClicked: resetRequested()

                background: Rectangle {
                    implicitWidth: 140
                    implicitHeight: 40
                    color: parent.down ? Qt.darker(surfaceColor, 1.1) : surfaceColor
                    radius: 8
                    border.color: dangerColor
                    border.width: 1
                }

                contentItem: Text {
                    text: parent.text
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: dangerColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
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
                text: qsTr("Apply")
                font.family: "Fira Sans"
                onClicked: {
                    var settings = collectSettings()
                    applyRequested(settings)
                    root.accept()
                }

                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    color: parent.down ? Qt.darker(ctaColor, 1.1) : ctaColor
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
    property int categoryIndex: 0

    function getCategoryTitle(index) {
        switch (index) {
            case 0: return qsTr("General Settings")
            case 1: return qsTr("Console Settings")
            case 2: return qsTr("Sessions Settings")
            case 3: return qsTr("Tasks Settings")
            case 4: return qsTr("Appearance Settings")
            default: return qsTr("Settings")
        }
    }

    function collectSettings() {
        return {
            theme: currentTheme,
            fontSize: fontSize,
            fontFamily: fontFamily,
            terminalFontSize: terminalFontSize,
            consoleTimestamp: consoleTimestamp,
            consoleAutoScroll: consoleAutoScroll,
            consoleNoWrap: consoleNoWrap,
            consoleShowBackground: consoleShowBackground,
            consoleTheme: consoleTheme
        }
    }

    // Public API
    function loadSettings(settings) {
        currentTheme = settings.theme || "Light"
        fontSize = settings.fontSize || 12
        fontFamily = settings.fontFamily || "Fira Code"
        terminalFontSize = settings.terminalFontSize || 12
        consoleTimestamp = settings.consoleTimestamp !== false
        consoleAutoScroll = settings.consoleAutoScroll !== false
        consoleNoWrap = settings.consoleNoWrap || false
        consoleShowBackground = settings.consoleShowBackground !== false
        consoleTheme = settings.consoleTheme || "Default"
    }

    function resetToDefaults() {
        currentTheme = "Light"
        fontSize = 12
        fontFamily = "Fira Code"
        terminalFontSize = 12
        consoleTimestamp = true
        consoleAutoScroll = true
        consoleNoWrap = false
        consoleShowBackground = true
        consoleTheme = "Default"
    }
}
