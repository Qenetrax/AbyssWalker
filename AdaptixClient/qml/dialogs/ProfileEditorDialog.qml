import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root

    title: qsTr("Malleable Profile Editor")
    modal: true
    standardButtons: Dialog.NoButton
    width: 800
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
    readonly property color codeBackgroundColor: "#1E293B"

    // Form Properties
    property string profileName: ""
    property string profileContent: ""
    property bool hasUnsavedChanges: false

    // Signals
    signal saveRequested(string name, string content)
    signal validateRequested(string content)

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
                text: qsTr("Malleable Profile Editor")
                font.family: "Fira Sans"
                font.pixelSize: 20
                font.weight: Font.Bold
                color: textColor
            }

            Rectangle {
                visible: hasUnsavedChanges
                width: 8
                height: 8
                radius: 4
                color: warningColor

                readonly property color warningColor: "#F59E0B"
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
                    onClicked: {
                        if (hasUnsavedChanges) {
                            unsavedDialog.open()
                        } else {
                            root.reject()
                        }
                    }
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
            spacing: 20

            // Left Panel - Editor
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

            // Profile Name
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Profile Name:")
                    font.family: "Fira Sans"
                    font.pixelSize: 13
                    color: textSecondaryColor
                }

                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: qsTr("Enter profile name")
                    font.family: "Fira Code"
                    font.pixelSize: 13
                    color: textColor
                    text: profileName
                    onTextChanged: {
                        profileName = text
                        hasUnsavedChanges = true
                    }

                    background: Rectangle {
                        color: surfaceColor
                        radius: 6
                        border.color: nameField.activeFocus ? primaryColor : borderColor
                        border.width: 1
                    }
                }

                Button {
                    text: qsTr("Load")
                    font.family: "Fira Sans"
                    onClicked: loadDialog.open()

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
                    text: qsTr("Import")
                    font.family: "Fira Sans"
                    onClicked: importDialog.open()

                    background: Rectangle {
                        implicitWidth: 70
                        implicitHeight: 36
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
            }

            // Code Editor
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: codeBackgroundColor
                radius: 8

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Editor Toolbar
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        color: Qt.darker(codeBackgroundColor, 1.2)
                        radius: 8

                        // Bottom corners not rounded
                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 8
                            color: parent.color
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text {
                                text: qsTr("Profile Configuration")
                                font.family: "Fira Sans"
                                font.pixelSize: 12
                                color: "#94A3B8"
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                text: qsTr("Line %1, Col %2").arg(lineCount).arg(colCount)
                                font.family: "Fira Code"
                                font.pixelSize: 11
                                color: "#64748B"
                            }
                        }
                    }

                    // Text Editor
                    Flickable {
                        id: editorFlickable
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: codeEdit.paintedWidth
                        contentHeight: codeEdit.paintedHeight

                        function ensureVisible(r) {
                            if (contentX >= r.x)
                                contentX = r.x
                            else if (contentX + width <= r.x + r.width)
                                contentX = r.x + r.width - width
                            if (contentY >= r.y)
                                contentY = r.y
                            else if (contentY + height <= r.y + r.height)
                                contentY = r.y + r.height - height
                        }

                        TextEdit {
                            id: codeEdit
                            width: editorFlickable.width
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: "#E2E8F0"
                            selectionColor: primaryColor
                            selectedTextColor: "white"
                            text: profileContent
                            onTextChanged: {
                                profileContent = text
                                hasUnsavedChanges = true
                            }
                            wrapMode: TextEdit.NoWrap
                            selectByMouse: true
                            onCursorRectangleChanged: editorFlickable.ensureVisible(cursorRectangle)

                            // Syntax highlighting placeholder
                            // In real implementation, you would use a syntax highlighter
                        }

                        // Line numbers
                        Column {
                            id: lineNumbers
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.leftMargin: -30
                            spacing: 0

                            Repeater {
                                model: lineCount
                                Text {
                                    text: index + 1
                                    font.family: "Fira Code"
                                    font.pixelSize: 13
                                    color: "#475569"
                                    horizontalAlignment: Text.AlignRight
                                    width: 30
                                }
                            }
                        }
                    }
                }
            }

            // Validation Status
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    visible: validationStatus !== ""
                    width: 8
                    height: 8
                    radius: 4
                    color: validationValid ? ctaColor : dangerColor
                }

                Text {
                    text: validationStatus
                    font.family: "Fira Code"
                    font.pixelSize: 12
                    color: validationValid ? ctaColor : dangerColor
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("Validate")
                    font.family: "Fira Sans"
                    onClicked: validateRequested(profileContent)

                    background: Rectangle {
                        implicitWidth: 90
                        implicitHeight: 32
                        color: parent.down ? Qt.darker(secondaryColor, 1.1) : secondaryColor
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

            // Right Panel - Help & Templates
            ColumnLayout {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                spacing: 12

                Text {
                    text: qsTr("Templates")
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

                    ListView {
                        anchors.fill: parent
                        anchors.margins: 8
                        clip: true
                        model: templateModel
                        spacing: 4

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 40
                            color: templateMouseArea.containsMouse ? backgroundColor : "transparent"
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 8

                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 6
                                    color: Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.1)

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.icon
                                        font.pixelSize: 12
                                        color: primaryColor
                                    }
                                }

                                Text {
                                    text: modelData.name
                                    font.family: "Fira Sans"
                                    font.pixelSize: 13
                                    color: textColor
                                }
                            }

                            MouseArea {
                                id: templateMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: loadTemplate(modelData.name)
                            }
                        }
                    }
                }

                Text {
                    text: qsTr("Quick Reference")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    color: surfaceColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 12

                        Text {
                            text: qsTr("Common blocks:\n\n• http-get\n• http-post\n• http-stager\n• dns-beacon\n• stage\n• post-ex\n\nUse 'set' for settings\nUse 'header' for HTTP headers")
                            font.family: "Fira Code"
                            font.pixelSize: 11
                            color: textSecondaryColor
                            wrapMode: Text.WordWrap
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
                text: qsTr("Export")
                font.family: "Fira Sans"
                onClicked: exportDialog.open()

                background: Rectangle {
                    implicitWidth: 90
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

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Cancel")
                font.family: "Fira Sans"
                onClicked: {
                    if (hasUnsavedChanges) {
                        unsavedDialog.open()
                    } else {
                        root.reject()
                    }
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
                text: qsTr("Save")
                font.family: "Fira Sans"
                enabled: profileName.trim() !== ""
                onClicked: {
                    saveRequested(profileName, profileContent)
                    hasUnsavedChanges = false
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
    property int lineCount: 1
    property int colCount: 1
    property string validationStatus: ""
    property bool validationValid: true

    property var templateModel: [
        { name: "HTTP Default", icon: "H" },
        { name: "HTTPS Default", icon: "S" },
        { name: "DNS Beacon", icon: "D" },
        { name: "SMB Beacon", icon: "B" },
        { name: "Custom", icon: "C" }
    ]

    function loadTemplate(name) {
        // Load template based on name
        if (name === "HTTP Default") {
            profileContent = "set sleeptime \"5000\";\nset jitter    \"0\";\n\nhttp-get {\n    set uri \"/api/v1/data\";\n    \n    client {\n        header \"Accept\" \"application/json\";\n    }\n    \n    server {\n        header \"Content-Type\" \"application/json\";\n    }\n}"
        }
        hasUnsavedChanges = true
    }

    function updateLineCount() {
        lineCount = profileContent.split('\n').length
    }

    // File Dialogs
    FileDialog {
        id: importDialog
        title: qsTr("Import Profile")
        onAccepted: {
            // Load profile from file
        }
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export Profile")
        fileMode: FileDialog.SaveFile
        onAccepted: {
            // Save profile to file
        }
    }

    Dialog {
        id: unsavedDialog
        title: qsTr("Unsaved Changes")
        modal: true
        standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel
        width: 360

        contentItem: Text {
            text: qsTr("You have unsaved changes. Do you want to save before closing?")
            font.family: "Fira Sans"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (profileName.trim() !== "") {
                saveRequested(profileName, profileContent)
                root.accept()
            }
        }

        onDiscarded: root.reject()
    }

    Dialog {
        id: loadDialog
        title: qsTr("Load Profile")
        modal: true
        width: 400
        height: 300

        contentItem: ListView {
            model: ["Profile 1", "Profile 2", "Profile 3"]
            delegate: Rectangle {
                width: parent.width
                height: 40
                color: loadMouseArea.containsMouse ? backgroundColor : "transparent"
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
                    id: loadMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        profileName = modelData
                        loadDialog.close()
                    }
                }
            }
        }
    }

    // Public API
    function setProfile(name, content) {
        profileName = name
        profileContent = content
        hasUnsavedChanges = false
    }

    function setValidationResult(valid, message) {
        validationValid = valid
        validationStatus = message
    }

    function reset() {
        profileName = ""
        profileContent = ""
        hasUnsavedChanges = false
        validationStatus = ""
        validationValid = true
    }
}
