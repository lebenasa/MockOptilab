import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QuickCam 1.0

ApplicationWindow {
    visible: true
    width: 1280
    height: 720
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
            Label {
                text: "(%1x%2)".arg(stepper.x).arg(stepper.y)
            }
        }
    }
    
    ScrollView {
        anchors.fill: parent
        clip: true
        Rectangle {
            color: "#222"
            width: serialcapture.cellSize.width * cammodel.cols
            height: serialcapture.cellSize.height * cammodel.rows
            GridView {
                id: cameraGrid
                model: cammodel
                anchors.fill: parent
                interactive: false
                cellWidth: serialcapture.cellSize.width
                cellHeight: serialcapture.cellSize.height
                delegate: Rectangle {
                    id: delegateBorder
                    width: cameraGrid.cellWidth
                    height: cameraGrid.cellHeight
                    color: "transparent"
                    border.width: 1
                    border.color: selected ? "yellow" : highlight ? "magenta" : "green"
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
            MouseArea {
                id: cameraMouse
                anchors.fill: parent
                hoverEnabled: true
                property int mousecount: 0
                onPositionChanged: {
                    if (mousecount === 15) {
                       serialcapture.procHighlight(Qt.point(mouse.x, mouse.y))
                       mousecount = 0
                    }
                    ++mousecount
                }
                onClicked: serialcapture.procSelect(Qt.point(mouse.x, mouse.y))
            }
        }
    }
}
