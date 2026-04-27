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
    signal settingsChanged()
    signal themeImported(string path)
    signal consoleThemeImported(string path)

    Rectangle {
        anchors.fill: parent
        color: backgroundColor

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // Sidebar
            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                color: surfaceColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 8

                    Text {
                        text: qsTr("Settings")
                        font.family: "Fira Sans"
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: textColor
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: borderColor
                    }

                    // Settings Categories
                    Repeater {
                        model: [
                            { name: qsTr("General"), icon: "gear" },
                            { name: qsTr("Console"), icon: "terminal" },
                            { name: qsTr("Sessions"), icon: "users" },
                            { name: qsTr("Tasks"), icon: "list" },
                            { name: qsTr("Appearance"), icon: "palette" }
                        ]

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            radius: 8
                            color: settingsList.currentIndex === index ? Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.1) : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 12

                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 6
                                    color: settingsList.currentIndex === index ? primaryColor : "transparent"
                                    border.color: settingsList.currentIndex === index ? primaryColor : borderColor
                                    border.width: 1
                                }

                                Text {
                                    text: modelData.name
                                    font.family: "Fira Sans"
                                    font.pixelSize: 14
                                    color: settingsList.currentIndex === index ? primaryColor : textColor
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: settingsList.currentIndex = index
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }

                // Right border
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

                StackLayout {
                    id: settingsStack
                    anchors.fill: parent
                    anchors.margins: 24
                    currentIndex: settingsList.currentIndex

                    // General Settings
                    ColumnLayout {
                        spacing: 24

                        Text {
                            text: qsTr("General Settings")
                            font.family: "Fira Sans"
                            font.pixelSize: 20
                            font.weight: Font.Bold
                            color: textColor
                        }

                        // Theme Selection
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: qsTr("Theme")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: textColor
                                    }

                                    Text {
                                        text: qsTr("Select the application color theme")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 12
                                        color: textSecondaryColor
                                    }
                                }

                                ComboBox {
                                    id: themeCombo
                                    Layout.preferredWidth: 200
                                    Layout.preferredHeight: 40
                                    model: ["Light", "Dark", "System"]
                                    currentIndex: model.indexOf(currentTheme)
                                    onCurrentTextChanged: currentTheme = currentText

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 8
                                        border.color: themeCombo.popup.visible ? primaryColor : borderColor
                                        border.width: 1
                                    }
                                }

                                Button {
                                    text: qsTr("Import")
                                    font.family: "Fira Sans"
                                    onClicked: themeImportDialog.open()

                                    background: Rectangle {
                                        implicitWidth: 80
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
                                        color: textColor
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }

                        // Font Size
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: qsTr("Font Size")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: textColor
                                    }

                                    Text {
                                        text: qsTr("Adjust the base font size for the interface")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 12
                                        color: textSecondaryColor
                                    }
                                }

                                SpinBox {
                                    id: fontSizeSpin
                                    Layout.preferredWidth: 120
                                    Layout.preferredHeight: 40
                                    from: 8
                                    to: 24
                                    value: fontSize
                                    onValueChanged: fontSize = value

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 8
                                        border.color: borderColor
                                        border.width: 1
                                    }
                                }
                            }
                        }

                        // Font Family
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: qsTr("Font Family")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: textColor
                                    }

                                    Text {
                                        text: qsTr("Select the primary font family")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 12
                                        color: textSecondaryColor
                                    }
                                }

                                ComboBox {
                                    id: fontFamilyCombo
                                    Layout.preferredWidth: 200
                                    Layout.preferredHeight: 40
                                    model: ["Fira Code", "Fira Sans", "Consolas", "Source Code Pro"]
                                    currentIndex: model.indexOf(fontFamily)
                                    onCurrentTextChanged: fontFamily = currentText

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 8
                                        border.color: fontFamilyCombo.popup.visible ? primaryColor : borderColor
                                        border.width: 1
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }

                    // Console Settings
                    ColumnLayout {
                        spacing: 24

                        Text {
                            text: qsTr("Console Settings")
                            font.family: "Fira Sans"
                            font.pixelSize: 20
                            font.weight: Font.Bold
                            color: textColor
                        }

                        // Console Options
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 16

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
                                        implicitWidth: 20
                                        implicitHeight: 20
                                        radius: 4
                                        color: parent.checked ? primaryColor : "transparent"
                                        border.color: parent.checked ? primaryColor : borderColor
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
                                        color: textColor
                                        leftPadding: parent.indicator.width + 12
                                    }
                                }

                                CheckBox {
                                    text: qsTr("Auto-scroll to bottom")
                                    checked: consoleAutoScroll
                                    onCheckedChanged: consoleAutoScroll = checked

                                    indicator: Rectangle {
                                        implicitWidth: 20
                                        implicitHeight: 20
                                        radius: 4
                                        color: parent.checked ? primaryColor : "transparent"
                                        border.color: parent.checked ? primaryColor : borderColor
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
                                        color: textColor
                                        leftPadding: parent.indicator.width + 12
                                    }
                                }

                                CheckBox {
                                    text: qsTr("No Wrap")
                                    checked: consoleNoWrap
                                    onCheckedChanged: consoleNoWrap = checked

                                    indicator: Rectangle {
                                        implicitWidth: 20
                                        implicitHeight: 20
                                        radius: 4
                                        color: parent.checked ? primaryColor : "transparent"
                                        border.color: parent.checked ? primaryColor : borderColor
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
                                        color: textColor
                                        leftPadding: parent.indicator.width + 12
                                    }
                                }

                                CheckBox {
                                    text: qsTr("Show Background Image")
                                    checked: consoleShowBackground
                                    onCheckedChanged: consoleShowBackground = checked

                                    indicator: Rectangle {
                                        implicitWidth: 20
                                        implicitHeight: 20
                                        radius: 4
                                        color: parent.checked ? primaryColor : "transparent"
                                        border.color: parent.checked ? primaryColor : borderColor
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
                                        color: textColor
                                        leftPadding: parent.indicator.width + 12
                                    }
                                }
                            }
                        }

                        // Console Theme
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: qsTr("Console Theme")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: textColor
                                    }

                                    Text {
                                        text: qsTr("Select the console color theme")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 12
                                        color: textSecondaryColor
                                    }
                                }

                                ComboBox {
                                    id: consoleThemeCombo
                                    Layout.preferredWidth: 200
                                    Layout.preferredHeight: 40
                                    model: ["Default", "Dark", "Light", "Solarized Dark", "Solarized Light"]
                                    currentIndex: model.indexOf(consoleTheme)
                                    onCurrentTextChanged: consoleTheme = currentText

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 8
                                        border.color: consoleThemeCombo.popup.visible ? primaryColor : borderColor
                                        border.width: 1
                                    }
                                }

                                Button {
                                    text: qsTr("Import")
                                    font.family: "Fira Sans"
                                    onClicked: consoleThemeImportDialog.open()

                                    background: Rectangle {
                                        implicitWidth: 80
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
                                        color: textColor
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }

                        // Terminal Font Size
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: qsTr("Terminal Font Size")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: textColor
                                    }

                                    Text {
                                        text: qsTr("Font size for terminal and console windows")
                                        font.family: "Fira Sans"
                                        font.pixelSize: 12
                                        color: textSecondaryColor
                                    }
                                }

                                SpinBox {
                                    id: terminalFontSizeSpin
                                    Layout.preferredWidth: 120
                                    Layout.preferredHeight: 40
                                    from: 8
                                    to: 24
                                    value: terminalFontSize
                                    onValueChanged: terminalFontSize = value

                                    background: Rectangle {
                                        color: backgroundColor
                                        radius: 8
                                        border.color: borderColor
                                        border.width: 1
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }

                    // Sessions Settings
                    ColumnLayout {
                        spacing: 24

                        Text {
                            text: qsTr("Sessions Settings")
                            font.family: "Fira Sans"
                            font.pixelSize: 20
                            font.weight: Font.Bold
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Session column visibility settings")
                                font.family: "Fira Sans"
                                font.pixelSize: 16
                                color: textSecondaryColor
                            }
                        }
                    }

                    // Tasks Settings
                    ColumnLayout {
                        spacing: 24

                        Text {
                            text: qsTr("Tasks Settings")
                            font.family: "Fira Sans"
                            font.pixelSize: 20
                            font.weight: Font.Bold
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Task column visibility settings")
                                font.family: "Fira Sans"
                                font.pixelSize: 16
                                color: textSecondaryColor
                            }
                        }
                    }

                    // Appearance Settings
                    ColumnLayout {
                        spacing: 24

                        Text {
                            text: qsTr("Appearance Settings")
                            font.family: "Fira Sans"
                            font.pixelSize: 20
                            font.weight: Font.Bold
                            color: textColor
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: surfaceColor
                            radius: 12
                            border.color: borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Tab blink and animation settings")
                                font.family: "Fira Sans"
                                font.pixelSize: 16
                                color: textSecondaryColor
                            }
                        }
                    }
                }
            }
        }
    }

    // Settings List (for index tracking)
    QtObject {
        id: settingsList
        property int currentIndex: 0
    }

    // Dialogs
    FileDialog {
        id: themeImportDialog
        title: qsTr("Import Theme")
        onAccepted: root.themeImported(selectedFile)
    }

    FileDialog {
        id: consoleThemeImportDialog
        title: qsTr("Import Console Theme")
        onAccepted: root.consoleThemeImported(selectedFile)
    }

    // Public API
    function loadSettings(settingsObj) {
        currentTheme = settingsObj.theme || "Light"
        fontSize = settingsObj.fontSize || 12
        fontFamily = settingsObj.fontFamily || "Fira Code"
        terminalFontSize = settingsObj.terminalFontSize || 12
        consoleTimestamp = settingsObj.consoleTimestamp !== false
        consoleAutoScroll = settingsObj.consoleAutoScroll !== false
        consoleNoWrap = settingsObj.consoleNoWrap || false
        consoleShowBackground = settingsObj.consoleShowBackground !== false
        consoleTheme = settingsObj.consoleTheme || "Default"
    }

    function getSettings() {
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
}
