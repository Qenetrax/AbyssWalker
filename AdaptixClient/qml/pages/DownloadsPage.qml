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
    signal cancelDownload(string downloadId)
    signal retryDownload(string downloadId)
    signal removeDownload(string downloadId)
    signal openDownload(string downloadId)

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
                        text: qsTr("Downloads")
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
                        placeholderText: qsTr("Search downloads...")
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

                    // Statistics
                    RowLayout {
                        spacing: 16

                        Rectangle {
                            Layout.preferredWidth: 140
                            Layout.preferredHeight: 36
                            color: backgroundColor
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: successColor
                                }

                                Text {
                                    text: qsTr("Completed: %1").arg(completedCount)
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                }
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 140
                            Layout.preferredHeight: 36
                            color: backgroundColor
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: warningColor
                                }

                                Text {
                                    text: qsTr("In Progress: %1").arg(inProgressCount)
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                }
                            }
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

            // Downloads List
            ListView {
                id: downloadsList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: downloadsModel
                spacing: 8
                leftMargin: 16
                rightMargin: 16
                topMargin: 16
                bottomMargin: 16

                delegate: Rectangle {
                    width: downloadsList.width - downloadsList.leftMargin - downloadsList.rightMargin
                    height: 80
                    color: surfaceColor
                    radius: 12
                    border.color: borderColor
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        // Status Icon
                        Rectangle {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            radius: 12
                            color: model.status === "completed" ? Qt.rgba(successColor.r, successColor.g, successColor.b, 0.1)
                                   : model.status === "in_progress" ? Qt.rgba(warningColor.r, warningColor.g, warningColor.b, 0.1)
                                   : Qt.rgba(dangerColor.r, dangerColor.g, dangerColor.b, 0.1)

                            Rectangle {
                                anchors.centerIn: parent
                                width: 24
                                height: 24
                                radius: 12
                                color: model.status === "completed" ? successColor
                                       : model.status === "in_progress" ? warningColor
                                       : dangerColor
                            }
                        }

                        // Info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: model.filename
                                font.family: "Fira Sans"
                                font.pixelSize: 15
                                font.weight: Font.Medium
                                color: textColor
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                spacing: 16

                                Text {
                                    text: qsTr("Size: %1").arg(model.size)
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }

                                Text {
                                    text: qsTr("Agent: %1").arg(model.agentId)
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }

                                Text {
                                    visible: model.status === "in_progress"
                                    text: qsTr("Speed: %1").arg(model.speed)
                                    font.family: "Fira Code"
                                    font.pixelSize: 12
                                    color: textSecondaryColor
                                }
                            }

                            // Progress bar
                            Rectangle {
                                visible: model.status === "in_progress"
                                Layout.fillWidth: true
                                Layout.preferredHeight: 8
                                color: backgroundColor
                                radius: 4

                                Rectangle {
                                    width: parent.width * (model.progress / 100)
                                    height: parent.height
                                    radius: 4
                                    color: primaryColor
                                    anchors.left: parent.left
                                    anchors.top: parent.top

                                    NumberAnimation on width {
                                        running: model.status === "in_progress"
                                        duration: 300
                                    }
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "%1%".arg(Math.round(model.progress))
                                    font.family: "Fira Code"
                                    font.pixelSize: 10
                                    color: textColor
                                }
                            }
                        }

                        // Action Buttons
                        RowLayout {
                            spacing: 8

                            Button {
                                visible: model.status === "in_progress"
                                text: qsTr("Cancel")
                                font.family: "Fira Sans"
                                onClicked: root.cancelDownload(model.downloadId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
                                    color: parent.down ? Qt.darker(dangerColor, 1.1) : dangerColor
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

                            Button {
                                visible: model.status === "failed"
                                text: qsTr("Retry")
                                font.family: "Fira Sans"
                                onClicked: root.retryDownload(model.downloadId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
                                    color: parent.down ? Qt.darker(primaryColor, 1.1) : primaryColor
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

                            Button {
                                visible: model.status === "completed"
                                text: qsTr("Open")
                                font.family: "Fira Sans"
                                onClicked: root.openDownload(model.downloadId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
                                    color: parent.down ? Qt.darker(ctaColor, 1.1) : ctaColor
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

                            Button {
                                visible: model.status !== "in_progress"
                                text: qsTr("Remove")
                                font.family: "Fira Sans"
                                onClicked: root.removeDownload(model.downloadId)

                                background: Rectangle {
                                    implicitWidth: 80
                                    implicitHeight: 32
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
                }

                // Empty state
                Text {
                    anchors.centerIn: parent
                    visible: downloadsModel.count === 0
                    text: qsTr("No downloads")
                    font.family: "Fira Sans"
                    font.pixelSize: 16
                    color: textSecondaryColor
                }
            }
        }
    }

    // Downloads Model
    ListModel {
        id: downloadsModel
    }

    property int completedCount: 0
    property int inProgressCount: 0

    // Public API
    function addDownload(downloadData) {
        downloadsModel.append(downloadData)
        updateCounts()
    }

    function updateDownload(downloadId, downloadData) {
        for (var i = 0; i < downloadsModel.count; i++) {
            if (downloadsModel.get(i).downloadId === downloadId) {
                downloadsModel.set(i, downloadData)
                break
            }
        }
        updateCounts()
    }

    function removeDownloadById(downloadId) {
        for (var i = 0; i < downloadsModel.count; i++) {
            if (downloadsModel.get(i).downloadId === downloadId) {
                downloadsModel.remove(i)
                break
            }
        }
        updateCounts()
    }

    function clearDownloads() {
        downloadsModel.clear()
        completedCount = 0
        inProgressCount = 0
    }

    function updateCounts() {
        var completed = 0
        var inProgress = 0
        for (var i = 0; i < downloadsModel.count; i++) {
            if (downloadsModel.get(i).status === "completed") {
                completed++
            } else if (downloadsModel.get(i).status === "in_progress") {
                inProgress++
            }
        }
        completedCount = completed
        inProgressCount = inProgress
    }
}