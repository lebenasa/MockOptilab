import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QuickCam 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("Tools")
            MenuItem {
                text: qsTr("Move")
                onTriggered: serialcapture.moveToSelected()
            }
        }
    }
    
    statusBar: StatusBar {
        RowLayout {
            
        }
    }
    
    ScrollView {
        anchors.fill: parent
        clip: true
        Rectangle {
            color: "#222"
            width: (serialcapture.cellSize.width + 1) * cammodel.cols
            height: (serialcapture.cellSize.height + 1) * cammodel.rows
            GridView {
                id: cameraGrid
                model: cammodel
                anchors.fill: parent
                interactive: false
                cellWidth: serialcapture.cellSize.width
                cellHeight: serialcapture.cellSize.height
                delegate: Rectangle {
                    id: delegateBorder
                    property bool highlighted
                    width: cameraGrid.cellWidth
                    height: cameraGrid.cellHeight
                    color: "transparent"
                    border.width: 1
                    border.color: selected ? "yellow" : highlighted ? "magenta" : "green"
                    CameraItem {
                        id: cameraDelegate
                        anchors.fill: parent
                        anchors.margins: 1
                        blocked: false
                        source: buffer
                        renderParams: CameraItem.ScaledToItem
                    }
                }
            }
        }
    }
}
