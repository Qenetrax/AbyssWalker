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
    readonly property color infoColor: "#3B82F6"

    property string filterText: ""
    property string levelFilter: "All"
    property bool autoScroll: true

    // Signals
    signal clearLogs()
    signal exportLogs()
    signal filterChanged(string filter, string level)

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
                        text: qsTr("Logs")
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
                        placeholderText: qsTr("Search logs...")
                        font.family: "Fira Sans"
                        font.pixelSize: 14
                        color: textColor
                        onTextChanged: {
                            filterText = text
                            root.filterChanged(filterText, levelFilter)
                        }

                        background: Rectangle {
                            color: backgroundColor
                            radius: 6
                            border.color: parent.activeFocus ? primaryColor : borderColor
                            border.width: 1
                        }
                    }

                    // Level Filter
                    ComboBox {
                        id: levelCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 36
                        model: ["All", "Info", "Warning", "Error", "Debug"]
                        currentIndex: 0
                        displayText: qsTr("Level: ") + currentText
                        onCurrentTextChanged: {
                            levelFilter = currentText
                            root.filterChanged(filterText, levelFilter)
                        }

                        background: Rectangle {
                            color: backgroundColor
                            radius: 6
                            border.color: levelCombo.popup.visible ? primaryColor : borderColor
                            border.width: 1
                        }
                    }

                    // Auto Scroll
                    CheckBox {
                        id: autoScrollCheck
                        text: qsTr("Auto-scroll")
                        checked: autoScroll
                        onCheckedChanged: autoScroll = checked

                        indicator: Rectangle {
                            implicitWidth: 18
                            implicitHeight: 18
                            radius: 4
                            color: autoScrollCheck.checked ? primaryColor : "transparent"
                            border.color: autoScrollCheck.checked ? primaryColor : borderColor
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                color: "white"
                                font.pixelSize: 12
                                visible: autoScrollCheck.checked
                            }
                        }

                        contentItem: Text {
                            text: autoScrollCheck.text
                            font.family: "Fira Sans"
                            font.pixelSize: 14
                            color: textColor
                            leftPadding: autoScrollCheck.indicator.width + 8
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Statistics
                    RowLayout {
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 36
                            color: Qt.rgba(infoColor.r, infoColor.g, infoColor.b, 0.1)
                            radius: 6

                            Text {
                                anchors.centerIn: parent
                                text: infoCount.toString()
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                                color: infoColor
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 36
                            color: Qt.rgba(warningColor.r, warningColor.g, warningColor.b, 0.1)
                            radius: 6

                            Text {
                                anchors.centerIn: parent
                                text: warningCount.toString()
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                                color: warningColor
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 36
                            color: Qt.rgba(dangerColor.r, dangerColor.g, dangerColor.b, 0.1)
                            radius: 6

                            Text {
                                anchors.centerIn: parent
                                text: errorCount.toString()
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                                color: dangerColor
                            }
                        }
                    }

                    // Separator
                    Rectangle {
                        width: 1
                        height: 32
                        color: borderColor
                    }

                    // Action Buttons
                    Button {
                        text: qsTr("Export")
                        font.family: "Fira Sans"
                        onClicked: root.exportLogs()

                        background: Rectangle {
                            implicitWidth: 80
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
                        text: qsTr("Clear")
                        font.family: "Fira Sans"
                        onClicked: root.clearLogs()

                        background: Rectangle {
                            implicitWidth: 80
                            implicitHeight: 36
                            color: parent.down ? Qt.darker(dangerColor, 1.1) : dangerColor
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

            // Log View
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: surfaceColor
                border.color: borderColor
                border.width: 1

                ListView {
                    id: logList
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    model: logsModel
                    spacing: 0

                    delegate: Rectangle {
                        width: logList.width
                        height: 36
                        color: row % 2 === 0 ? surfaceColor : backgroundColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 16

                            // Level indicator
                            Rectangle {
                                width: 4
                                height: 24
                                radius: 2
                                color: model.level === "Error" ? dangerColor
                                       : model.level === "Warning" ? warningColor
                                       : model.level === "Debug" ? textSecondaryColor
                                       : infoColor
                            }

                            // Timestamp
                            Text {
                                text: model.timestamp
                                font.family: "Fira Code"
                                font.pixelSize: 12
                                color: textSecondaryColor
                                Layout.preferredWidth: 140
                            }

                            // Source
                            Text {
                                text: model.source
                                font.family: "Fira Code"
                                font.pixelSize: 12
                                color: primaryColor
                                Layout.preferredWidth: 100
                                elide: Text.ElideRight
                            }

                            // Message
                            Text {
                                text: model.message
                                font.family: "Fira Code"
                                font.pixelSize: 13
                                color: textColor
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 1
                            color: borderColor
                            opacity: 0.3
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            onClicked: function(mouse) {
                                logContextMenu.modelData = model
                                logContextMenu.popup()
                            }
                        }
                    }

                    onCountChanged: {
                        if (autoScroll && count > 0) {
                            positionViewAtEnd()
                        }
                    }
                }

                // Empty state
                Text {
                    anchors.centerIn: parent
                    visible: logsModel.count === 0
                    text: qsTr("No log entries")
                    font.family: "Fira Sans"
                    font.pixelSize: 16
                    color: textSecondaryColor
                }
            }
        }
    }

    // Context Menu
    Menu {
        id: logContextMenu
        property var modelData: null

        MenuItem {
            text: qsTr("Copy Message")
            onTriggered: {
                if (logContextMenu.modelData) {
                    // Copy to clipboard
                }
            }
        }

        MenuItem {
            text: qsTr("Copy Full Line")
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Filter by Source")
        }
    }

    // Logs Model
    ListModel {
        id: logsModel
    }

    property int infoCount: 0
    property int warningCount: 0
    property int errorCount: 0

    // Public API
    function addLog(timestamp, level, source, message) {
        logsModel.append({
            "timestamp": timestamp,
            "level": level,
            "source": source,
            "message": message
        })
        updateCounts()
    }

    function addLogEntry(entry) {
        logsModel.append(entry)
        updateCounts()
    }

    function clearAllLogs() {
        logsModel.clear()
        infoCount = 0
        warningCount = 0
        errorCount = 0
    }

    function updateCounts() {
        var info = 0, warn = 0, err = 0
        for (var i = 0; i < logsModel.count; i++) {
            var level = logsModel.get(i).level
            if (level === "Info") info++
            else if (level === "Warning") warn++
            else if (level === "Error") err++
        }
        infoCount = info
        warningCount = warn
        errorCount = err
    }

    function scrollToBottom() {
        logList.positionViewAtEnd()
    }
}
