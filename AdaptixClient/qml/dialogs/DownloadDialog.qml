import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root

    title: qsTr("Download File")
    modal: true
    standardButtons: Dialog.NoButton
    width: 520
    height: 400

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
    property string remotePath: ""
    property string localPath: ""
    property string agentId: ""
    property bool downloading: false
    property real progress: 0
    property string speed: ""

    // Signals
    signal downloadRequested(string remote, string local, string agent)
    signal cancelRequested()

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
                text: qsTr("Download File")
                font.family: "Fira Sans"
                font.pixelSize: 20
                font.weight: Font.Bold
                color: textColor
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                visible: !downloading
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

            // Remote File Path
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Remote File")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                TextField {
                    id: remotePathField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    placeholderText: qsTr("File path on agent (e.g., C:\\temp\\data.txt)")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: remotePath
                    onTextChanged: remotePath = text
                    enabled: !downloading

                    background: Rectangle {
                        color: surfaceColor
                        radius: 8
                        border.color: remotePathField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }
            }

            // Local Save Path
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Local Save Location")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    TextField {
                        id: localPathField
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        placeholderText: qsTr("Save location...")
                        font.family: "Fira Code"
                        font.pixelSize: 13
                        color: textColor
                        text: localPath
                        onTextChanged: localPath = text
                        readOnly: true

                        background: Rectangle {
                            color: surfaceColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1
                        }
                    }

                    Button {
                        text: qsTr("Browse")
                        font.family: "Fira Sans"
                        enabled: !downloading
                        onClicked: saveDialog.open()

                        background: Rectangle {
                            implicitWidth: 90
                            implicitHeight: 44
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

            // Agent Selection
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Source Agent")
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
                        enabled: !downloading
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

                                    Text {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        text: modelData.id + " - " + modelData.hostname
                                        font.family: "Fira Code"
                                        font.pixelSize: 13
                                        color: textColor
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    MouseArea {
                                        id: agentMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            agentId = modelData.id
                                            agentCombo.close()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Progress
            ColumnLayout {
                Layout.fillWidth: true
                visible: downloading
                spacing: 12

                Text {
                    text: qsTr("Downloading...")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 12
                    color: borderColor
                    radius: 6

                    Rectangle {
                        width: parent.width * (progress / 100)
                        height: parent.height
                        radius: 6
                        color: primaryColor

                        Behavior on width {
                            NumberAnimation { duration: 100 }
                        }
                    }
                }

                RowLayout {
                    spacing: 16

                    Text {
                        text: Math.round(progress) + "%"
                        font.family: "Fira Code"
                        font.pixelSize: 13
                        color: textSecondaryColor
                    }

                    Text {
                        visible: speed !== ""
                        text: speed
                        font.family: "Fira Code"
                        font.pixelSize: 13
                        color: textSecondaryColor
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
                text: downloading ? qsTr("Cancel") : qsTr("Close")
                font.family: "Fira Sans"
                onClicked: {
                    if (downloading) {
                        cancelRequested()
                    }
                    root.reject()
                }

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
                visible: !downloading
                text: qsTr("Download")
                font.family: "Fira Sans"
                enabled: isFormValid()
                onClicked: {
                    downloading = true
                    downloadRequested(remotePath, localPath, agentId)
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

    // Save Dialog
    FileDialog {
        id: saveDialog
        title: qsTr("Save Downloaded File")
        fileMode: FileDialog.SaveFile
        onAccepted: {
            localPath = selectedFile.toString().replace("file:///", "")
        }
    }

    // Internal Properties
    property var agentModel: []

    function isFormValid() {
        return remotePath !== "" && localPath !== "" && agentId !== ""
    }

    // Public API
    function setAgents(agents) {
        agentModel = agents
    }

    function setProgress(value, speedText) {
        progress = value
        speed = speedText || ""
    }

    function setComplete() {
        downloading = false
        progress = 100
    }

    function reset() {
        remotePath = ""
        localPath = ""
        agentId = ""
        downloading = false
        progress = 0
        speed = ""
    }
}