// Theme Interface
import QtQuick

QtObject {
    // Colors
    property color primaryColor: "#0369A1"
    property color secondaryColor: "#0EA5E9"
    property color ctaColor: "#22C55E"
    property color backgroundColor: "#F0F9FF"
    property color surfaceColor: "#FFFFFF"
    property color textColor: "#0C4A6E"
    property color textSecondaryColor: "#64748B"
    property color borderColor: "#E2E8F0"
    property color errorColor: "#EF4444"
    property color warningColor: "#F59E0B"
    property color successColor: "#22C55E"

    // Spacing
    property int spacingXS: 4
    property int spacingS: 8
    property int spacingM: 16
    property int spacingL: 24
    property int spacingXL: 32

    // Radius
    property int radiusS: 4
    property int radiusM: 8
    property int radiusL: 12

    // Fonts
    property string fontFamily: "Fira Sans"
    property string fontMono: "Fira Code"

    // Font Sizes
    property int fontSizeXS: 11
    property int fontSizeS: 12
    property int fontSizeM: 14
    property int fontSizeL: 16
    property int fontSizeXL: 20
    property int fontSizeXXL: 24

    // Animation
    property int animationDuration: 200

    // Shadows
    property string shadowSmall: "0 1px 3px rgba(0,0,0,0.1)"
    property string shadowMedium: "0 4px 6px rgba(0,0,0,0.1)"
    property string shadowLarge: "0 10px 15px rgba(0,0,0,0.1)"
}
