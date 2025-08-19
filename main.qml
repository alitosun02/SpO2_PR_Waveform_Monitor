import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    width: 600
    height: 450
    visible: true
    title: "SpO2 ve PR Gösterim"

    Column {
        anchors.centerIn: parent
        spacing: 20

        // SPO2 yazı
        Text {
            id: spo2Text
            text: reader.spo2 === -1 ? "SpO2: -" : "SpO2: " + reader.spo2 + " %"
            font.pixelSize: 20
        }

        // SPO2 çubuk grafiği
        Rectangle {
            width: 200
            height: 150
            color: "lightgray"

            Rectangle {
                width: parent.width
                height: reader.spo2
                anchors.bottom: parent.bottom
                color: "red"
            }
        }

        // PR yazı
        Text {
            id: prText
            text: reader.pr === -1 ? "PR: -" : "PR: " + reader.pr + " bpm"
            font.pixelSize: 20
        }

        // PR çubuk grafiği
        Rectangle {
            width: 200
            height: 150
            color: "lightgray"

            Rectangle {
                width: parent.width
                height: reader.pr
                anchors.bottom: parent.bottom
                color: "blue"
            }
        }
    }
}
