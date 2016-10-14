import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.1
import SceneGraphRendering 1.0

import "qrc:/qml/components"

Item {

    GMlibSceneRenderer
    {
        id: renderer

        anchors.fill: parent

        FpsBox
        {
            fps: renderer.fps

            anchors
            {
                margins: 20
                top:     parent.top
                right:   parent.right
            }

            width:  100
            height: 25
        }

        ComboBox {
          id: saveload
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.margins: 5

          width: 200

          opacity: 0.7

          model: [ "Menu", "Save", "Load"]
          //currentIndex: -1

          //onCurrentIndexChanged: console.debug(currentIndex)
          onCurrentIndexChanged: toggle()

          function toggle() {
              if (currentIndex === 1)
              {
                  scenario.clicksave()
                  currentIndex = 0
              }
              if (currentIndex === 2)
              {
                  scenario.clickload()
                  currentIndex = 0
              }
          }
        }

    }
}

