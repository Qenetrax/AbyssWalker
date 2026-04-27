// AdaptixC2 - Main Entry Point
// Light Theme Modern UI

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "styles"
import "components"
import "pages"
import "dialogs"

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1400
    height: 900
    minimumWidth: 1200
    minimumHeight: 700
    title: "AdaptixC2 - Red Team Operations Platform"

    // Theme
    property Theme theme: LightTheme {}

    // Global State
    property bool connected: false
    property string serverUrl: ""
    property string currentUser: ""
    property int onlineAgents: 0
    property int activeListeners: 0

    // Background
    color: theme.backgroundColor

    // Main Layout
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Side Navigation
        SideNav {
            id: sideNav
            Layout.fillHeight: true
            Layout.preferredWidth: 220
            theme: mainWindow.theme
            currentPage: pageStack.currentItem ? pageStack.currentItem.pageName : "dashboard"
            onPageSelected: function(page) {
                pageStack.replace(page + ".qml", { theme: mainWindow.theme })
            }
        }

        // Main Content Area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Top Bar
            TopBar {
                id: topBar
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                theme: mainWindow.theme
                connected: mainWindow.connected
                serverUrl: mainWindow.serverUrl
                currentUser: mainWindow.currentUser
            }

            // Page Stack
            StackView {
                id: pageStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                initialItem: "pages/DashboardPage.qml"

                pushEnter: Transition {
                    PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
                }
                pushExit: Transition {
                    PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
                }
                popEnter: Transition {
                    PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
                }
                popExit: Transition {
                    PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
                }
            }

            // Status Bar
            StatusBar {
                id: statusBar
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                theme: mainWindow.theme
                onlineAgents: mainWindow.onlineAgents
                activeListeners: mainWindow.activeListeners
            }
        }
    }

    // Connection Dialog (shown on startup if not connected)
    ConnectDialog {
        id: connectDialog
        anchors.centerIn: parent
        theme: mainWindow.theme
        visible: !mainWindow.connected
        onConnect: function(url, password) {
            Backend.connectToServer(url, password)
        }
    }

    // Connections
    Connections {
        target: Backend

        function onConnected() {
            mainWindow.connected = true
            mainWindow.serverUrl = Backend.serverUrl
            mainWindow.currentUser = Backend.currentUser
        }

        function onDisconnected() {
            mainWindow.connected = false
        }

        function onAgentsUpdated(count) {
            mainWindow.onlineAgents = count
        }

        function onListenersUpdated(count) {
            mainWindow.activeListeners = count
        }
    }
}
