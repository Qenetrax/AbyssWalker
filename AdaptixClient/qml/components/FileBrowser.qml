// File Browser Component
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property var theme
    property string agentId
    property string currentPath: "/"

    color: theme.surfaceColor

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: theme.backgroundColor

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8

                // Back Button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 4
                    color: backMouse.containsMouse ? theme.primaryColor + "20" : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "←"
                        font.family: theme.fontMono
                        font.pixelSize: 16
                        color: theme.textColor
                    }

                    MouseArea {
                        id: backMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: navigateUp()
                    }
                }

                // Path Display
                Rectangle {
                    Layout.fillWidth: true
                    height: 32
                    radius: 4
                    color: theme.surfaceColor
                    border.width: 1
                    border.color: theme.borderColor

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: currentPath
                        font.family: theme.fontMono
                        font.pixelSize: theme.fontSizeS
                        color: theme.textColor
                    }
                }

                // Refresh Button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 4
                    color: refreshMouse.containsMouse ? theme.primaryColor + "20" : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "↻"
                        font.family: theme.fontMono
                        font.pixelSize: 16
                        color: theme.textColor
                    }

                    MouseArea {
                        id: refreshMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: refreshDirectory()
                    }
                }

                // Upload Button
                Button {
                    text: "Upload"
                    font.family: theme.fontFamily
                    font.pixelSize: theme.fontSizeS

                    background: Rectangle {
                        color: parent.down ? theme.primaryColor + "CC" : parent.hovered ? theme.primaryColor + "DD" : theme.primaryColor
                        radius: 4
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: uploadDialog.open()
                }
            }
        }

        // File List
        ListView {
            id: fileListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: fileModel
            clip: true

            header: Rectangle {
                width: fileListView.width
                height: 36
                color: theme.backgroundColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16

                    Text {
                        text: "Name"
                        font.family: theme.fontFamily
                        font.pixelSize: theme.fontSizeS
                        font.bold: true
                        color: theme.textSecondaryColor
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "Size"
                        font.family: theme.fontFamily
                        font.pixelSize: theme.fontSizeS
                        font.bold: true
                        color: theme.textSecondaryColor
                        Layout.preferredWidth: 100
                    }

                    Text {
                        text: "Modified"
                        font.family: theme.fontFamily
                        font.pixelSize: theme.fontSizeS
                        font.bold: true
                        color: theme.textSecondaryColor
                        Layout.preferredWidth: 150
                    }
                }
            }

            delegate: Rectangle {
                width: fileListView.width
                height: 44
                color: rowMouse.containsMouse ? theme.primaryColor + "08" : (index % 2 === 0 ? theme.surfaceColor : theme.backgroundColor)

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16

                    // Icon
                    Text {
                        text: model.isDirectory ? "📁" : "📄"
                        font.family: theme.fontMono
                        font.pixelSize: 16
                        Layout.preferredWidth: 24
                    }

                    // Name
                    Text {
                        text: model.name
                        font.family: theme.fontFamily
                        font.pixelSize: theme.fontSizeS
                        color: theme.textColor
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // Size
                    Text {
                        text: model.isDirectory ? "" : formatSize(model.size)
                        font.family: theme.fontMono
                        font.pixelSize: theme.fontSizeS
                        color: theme.textSecondaryColor
                        Layout.preferredWidth: 100
                    }

                    // Modified
                    Text {
                        text: model.modified
                        font.family: theme.fontMono
                        font.pixelSize: theme.fontSizeS
                        color: theme.textSecondaryColor
                        Layout.preferredWidth: 150
                    }
                }

                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onDoubleClicked: {
                        if (model.isDirectory) {
                            currentPath = model.path
                            refreshDirectory()
                        } else {
                            downloadFile(model.path)
                        }
                    }

                    onClicked: fileListView.currentIndex = index

                    acceptedButtons: Qt.RightButton
                    onClicked: fileContextMenu.popup()
                }

                Menu {
                    id: fileContextMenu
                    MenuItem { text: "Download"; onTriggered: downloadFile(model.path) }
                    MenuItem { text: "Delete"; onTriggered: deleteFile(model.path) }
                    MenuSeparator {}
                    MenuItem { text: "Properties" }
                }
            }
        }
    }

    ListModel {
        id: fileModel
        // Populated from C++
    }

    function refreshDirectory() {
        Backend.listFiles(agentId, currentPath, function(files) {
            fileModel.clear()
            for (var i = 0; i < files.length; i++) {
                fileModel.append(files[i])
            }
        })
    }

    function navigateUp() {
        var parts = currentPath.split("/")
        if (parts.length > 1) {
            parts.pop()
            currentPath = parts.join("/") || "/"
            refreshDirectory()
        }
    }

    function downloadFile(path) {
        Backend.downloadFile(agentId, path)
    }

    function deleteFile(path) {
        Backend.deleteFile(agentId, path, function() {
            refreshDirectory()
        })
    }

    function formatSize(bytes) {
        if (bytes < 1024) return bytes + " B"
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB"
        if (bytes < 1024 * 1024 * 1024) return (bytes / 1024 / 1024).toFixed(1) + " MB"
        return (bytes / 1024 / 1024 / 1024).toFixed(1) + " GB"
    }

    Component.onCompleted: {
        refreshDirectory()
    }
}
