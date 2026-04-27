import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root

    title: qsTr("Connect to Server")
    modal: true
    standardButtons: Dialog.NoButton
    width: 560
    height: 520

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
    property string username: ""
    property string password: ""
    property string serverUrl: ""
    property string projectName: ""
    property string projectDir: ""
    property bool isNewProject: true

    // Signals
    signal connectRequested(string url, string user, string pass, string project, string dir)
    signal loadProfile(string profileName)

    background: Rectangle {
        color: surfaceColor
        radius: 12
        border.color: borderColor
        border.width: 1

        // Drop shadow effect
        layer.enabled: true
    }

    header: Rectangle {
        height: 60
        color: surfaceColor
        radius: 12

        // Bottom corners not rounded
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
                text: qsTr("Connect to Server")
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

            // Server Details
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Server Details")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
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
                            text: qsTr("Server URL:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                            Layout.preferredWidth: 100
                        }

                        TextField {
                            id: urlField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: "ws://localhost:5000"
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            text: serverUrl
                            onTextChanged: serverUrl = text

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: urlField.activeFocus ? primaryColor : borderColor
                                border.width: 1
                            }
                        }

                        Text {
                            text: qsTr("Username:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                        }

                        TextField {
                            id: usernameField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: qsTr("Enter username")
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            text: username
                            onTextChanged: username = text

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: usernameField.activeFocus ? primaryColor : borderColor
                                border.width: 1
                            }
                        }

                        Text {
                            text: qsTr("Password:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                        }

                        TextField {
                            id: passwordField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: qsTr("Enter password")
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            echoMode: TextInput.Password
                            text: password
                            onTextChanged: password = text

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: passwordField.activeFocus ? primaryColor : borderColor
                                border.width: 1
                            }
                        }
                    }
                }
            }

            // Project Settings
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: qsTr("Project Settings")
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    color: textColor
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: surfaceColor
                    radius: 8
                    border.color: borderColor
                    border.width: 1

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        columns: 3
                        columnSpacing: 12
                        rowSpacing: 12

                        Text {
                            text: qsTr("Project Name:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                            Layout.preferredWidth: 100
                        }

                        TextField {
                            id: projectNameField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: qsTr("Enter project name")
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            text: projectName
                            onTextChanged: projectName = text

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: projectNameField.activeFocus ? primaryColor : borderColor
                                border.width: 1
                            }
                        }

                        Item { Layout.preferredWidth: 1 }

                        Text {
                            text: qsTr("Project Directory:")
                            font.family: "Fira Sans"
                            font.pixelSize: 13
                            color: textSecondaryColor
                        }

                        TextField {
                            id: projectDirField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            placeholderText: qsTr("Select directory...")
                            font.family: "Fira Code"
                            font.pixelSize: 13
                            color: textColor
                            text: projectDir
                            onTextChanged: projectDir = text
                            readOnly: true

                            background: Rectangle {
                                color: backgroundColor
                                radius: 6
                                border.color: borderColor
                                border.width: 1
                            }
                        }

                        Button {
                            text: qsTr("Browse")
                            font.family: "Fira Sans"
                            onClicked: folderDialog.open()

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
                                font.pixelSize: 13
                                color: textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }

            // Profile Selection
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: qsTr("New Profile")
                    font.family: "Fira Sans"
                    onClicked: {
                        isNewProject = true
                        clearForm()
                    }

                    background: Rectangle {
                        implicitWidth: 100
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
                    text: qsTr("Load")
                    font.family: "Fira Sans"
                    onClicked: profileDialog.open()

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
                        font.pixelSize: 13
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: qsTr("Save")
                    font.family: "Fira Sans"
                    onClicked: saveProfile()

                    background: Rectangle {
                        implicitWidth: 80
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

                Item { Layout.fillWidth: true }
            }

            Item { Layout.fillHeight: true }
        }
    }

    footer: Rectangle {
        height: 72
        color: surfaceColor

        // Top corners not rounded
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
                text: qsTr("Connect")
                font.family: "Fira Sans"
                enabled: isFormValid()
                onClicked: {
                    root.connectRequested(serverUrl, username, password, projectName, projectDir)
                    root.accept()
                }

                background: Rectangle {
                    implicitWidth: 100
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

    // Validation
    property string errorMessage: ""

    function isFormValid() {
        if (serverUrl.trim() === "") return false
        if (username.trim() === "") return false
        if (password.trim() === "") return false
        if (projectName.trim() === "") return false
        if (projectDir.trim() === "") return false
        return true
    }

    function validateForm() {
        errorMessage = ""
        if (serverUrl.trim() === "") {
            errorMessage = qsTr("Server URL is required")
            return false
        }
        if (username.trim() === "") {
            errorMessage = qsTr("Username is required")
            return false
        }
        if (password.trim() === "") {
            errorMessage = qsTr("Password is required")
            return false
        }
        if (projectName.trim() === "") {
            errorMessage = qsTr("Project name is required")
            return false
        }
        if (projectDir.trim() === "") {
            errorMessage = qsTr("Project directory is required")
            return false
        }
        return true
    }

    function clearForm() {
        serverUrl = ""
        username = ""
        password = ""
        projectName = ""
        projectDir = ""
        errorMessage = ""
    }

    function saveProfile() {
        // Save profile logic
    }

    // Folder Dialog
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Project Directory")
        onAccepted: {
            projectDir = selectedFolder
        }
    }

    // Profile Dialog
    Dialog {
        id: profileDialog
        title: qsTr("Load Profile")
        modal: true
        width: 400
        height: 300
    }
}
