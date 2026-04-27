import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

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

    // Statistics Data
    property int totalAgents: 0
    property int onlineAgents: 0
    property int activeListeners: 0
    property int totalCredentials: 0
    property int totalTargets: 0

    // Signals
    signal refreshRequested()
    signal navigateToAgents()
    signal navigateToListeners()
    signal navigateToCredentials()
    signal navigateToTargets()

    Rectangle {
        anchors.fill: parent
        color: backgroundColor

        ScrollView {
            id: scrollView
            anchors.fill: parent
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 24
                spacing: 24

                // Header
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Text {
                        text: qsTr("Dashboard")
                        font.family: "Fira Sans"
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        color: textColor
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: qsTr("Refresh")
                        font.family: "Fira Sans"
                        icon.source: "qrc:/icons/refresh.svg"
                        onClicked: root.refreshRequested()

                        background: Rectangle {
                            color: parent.down ? Qt.darker(surfaceColor, 1.1) : surfaceColor
                            radius: 8
                            border.color: borderColor
                            border.width: 1
                        }

                        contentItem: RowLayout {
                            spacing: 8
                            Text {
                                text: parent.parent.text
                                font.family: "Fira Sans"
                                color: textColor
                            }
                        }
                    }
                }

                // Statistics Cards
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    // Online Agents Card
                    StatCard {
                        Layout.fillWidth: true
                        title: qsTr("Online Agents")
                        value: onlineAgents.toString()
                        subtitle: qsTr("%1 total").arg(totalAgents)
                        iconSource: "qrc:/icons/users.svg"
                        accentColor: successColor
                        onClicked: root.navigateToAgents()
                    }

                    // Active Listeners Card
                    StatCard {
                        Layout.fillWidth: true
                        title: qsTr("Active Listeners")
                        value: activeListeners.toString()
                        subtitle: qsTr("listening")
                        iconSource: "qrc:/icons/radio.svg"
                        accentColor: primaryColor
                        onClicked: root.navigateToListeners()
                    }

                    // Credentials Card
                    StatCard {
                        Layout.fillWidth: true
                        title: qsTr("Credentials")
                        value: totalCredentials.toString()
                        subtitle: qsTr("stored")
                        iconSource: "qrc:/icons/key.svg"
                        accentColor: warningColor
                        onClicked: root.navigateToCredentials()
                    }

                    // Targets Card
                    StatCard {
                        Layout.fillWidth: true
                        title: qsTr("Targets")
                        value: totalTargets.toString()
                        subtitle: qsTr("tracked")
                        iconSource: "qrc:/icons/target.svg"
                        accentColor: secondaryColor
                        onClicked: root.navigateToTargets()
                    }
                }

                // Main Content Area
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    // Recent Activity
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 400
                        color: surfaceColor
                        radius: 12
                        border.color: borderColor
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 16

                            Text {
                                text: qsTr("Recent Activity")
                                font.family: "Fira Sans"
                                font.pixelSize: 18
                                font.weight: Font.DemiBold
                                color: textColor
                            }

                            ListView {
                                id: activityList
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true
                                model: activityModel
                                spacing: 8

                                delegate: Rectangle {
                                    width: activityList.width
                                    height: 60
                                    color: mouseArea.containsMouse ? Qt.lighter(backgroundColor, 1.5) : "transparent"
                                    radius: 8

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        spacing: 12

                                        Rectangle {
                                            width: 40
                                            height: 40
                                            radius: 20
                                            color: model.iconBgColor

                                            Text {
                                                anchors.centerIn: parent
                                                text: model.iconText
                                                color: "white"
                                                font.family: "Fira Code"
                                                font.pixelSize: 16
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Text {
                                                text: model.title
                                                font.family: "Fira Sans"
                                                font.pixelSize: 14
                                                color: textColor
                                                elide: Text.ElideRight
                                            }

                                            Text {
                                                text: model.subtitle
                                                font.family: "Fira Sans"
                                                font.pixelSize: 12
                                                color: textSecondaryColor
                                                elide: Text.ElideRight
                                            }
                                        }

                                        Text {
                                            text: model.timestamp
                                            font.family: "Fira Code"
                                            font.pixelSize: 12
                                            color: textSecondaryColor
                                        }
                                    }

                                    MouseArea {
                                        id: mouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }
                                }
                            }

                            ListModel {
                                id: activityModel
                                // Sample data - will be populated from C++
                            }
                        }
                    }

                    // Quick Actions Panel
                    Rectangle {
                        Layout.preferredWidth: 280
                        Layout.preferredHeight: 400
                        color: surfaceColor
                        radius: 12
                        border.color: borderColor
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 16

                            Text {
                                text: qsTr("Quick Actions")
                                font.family: "Fira Sans"
                                font.pixelSize: 18
                                font.weight: Font.DemiBold
                                color: textColor
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                QuickActionButton {
                                    Layout.fillWidth: true
                                    text: qsTr("Create Listener")
                                    iconSource: "qrc:/icons/radio.svg"
                                    accentColor: primaryColor
                                    onClicked: Qt.createComponent("qrc:/qml/dialogs/ListenerDialog.qml").createObject(root)
                                }

                                QuickActionButton {
                                    Layout.fillWidth: true
                                    text: qsTr("Generate Agent")
                                    iconSource: "qrc:/icons/code.svg"
                                    accentColor: successColor
                                    onClicked: Qt.createComponent("qrc:/qml/dialogs/AgentBuilderDialog.qml").createObject(root)
                                }

                                QuickActionButton {
                                    Layout.fillWidth: true
                                    text: qsTr("Add Credential")
                                    iconSource: "qrc:/icons/key.svg"
                                    accentColor: warningColor
                                    onClicked: Qt.createComponent("qrc:/qml/dialogs/CredentialDialog.qml").createObject(root)
                                }

                                QuickActionButton {
                                    Layout.fillWidth: true
                                    text: qsTr("Add Target")
                                    iconSource: "qrc:/icons/target.svg"
                                    accentColor: secondaryColor
                                    onClicked: Qt.createComponent("qrc:/qml/dialogs/TargetDialog.qml").createObject(root)
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }
        }
    }

    // Stat Card Component
    component StatCard: Rectangle {
        property string title
        property string value
        property string subtitle
        property string iconSource
        property color accentColor

        signal clicked()

        height: 140
        color: surfaceColor
        radius: 12
        border.color: borderColor
        border.width: 1

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 4
            radius: 2
            color: accentColor
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Rectangle {
                width: 56
                height: 56
                radius: 12
                color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.1)

                Text {
                    anchors.centerIn: parent
                    text: value.charAt(0)
                    font.family: "Fira Code"
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    color: accentColor
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: title
                    font.family: "Fira Sans"
                    font.pixelSize: 14
                    color: textSecondaryColor
                }

                Text {
                    text: value
                    font.family: "Fira Code"
                    font.pixelSize: 32
                    font.weight: Font.Bold
                    color: textColor
                }

                Text {
                    text: subtitle
                    font.family: "Fira Sans"
                    font.pixelSize: 12
                    color: textSecondaryColor
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }

    // Quick Action Button Component
    component QuickActionButton: Rectangle {
        property string text
        property string iconSource
        property color accentColor

        signal clicked()

        height: 48
        radius: 8
        color: mouseArea.containsMouse ? Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.1) : backgroundColor

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            Rectangle {
                width: 28
                height: 28
                radius: 6
                color: accentColor
            }

            Text {
                text: parent.parent.text
                font.family: "Fira Sans"
                font.pixelSize: 14
                font.weight: Font.Medium
                color: textColor
            }

            Item { Layout.fillWidth: true }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }

    // Public API
    function updateStats(agents, online, listeners, creds, targets) {
        totalAgents = agents
        onlineAgents = online
        activeListeners = listeners
        totalCredentials = creds
        totalTargets = targets
    }

    function addActivityItem(iconText, iconBgColor, title, subtitle, timestamp) {
        activityModel.insert(0, {
            "iconText": iconText,
            "iconBgColor": iconBgColor,
            "title": title,
            "subtitle": subtitle,
            "timestamp": timestamp
        })
    }

    function clearActivity() {
        activityModel.clear()
    }
}
