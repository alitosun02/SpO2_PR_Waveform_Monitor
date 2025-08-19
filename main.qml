import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    width: 400
    height: 300
    visible: true
    title: "Biolight SPO2 packet"

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: reader.spo2 === -1 ? "SpO2: Geçersiz" : "SpO2: " + reader.spo2 + " %"
            font.pixelSize: 24
        }

        Text {
            text: reader.pr === -1 ? "PR: Geçersiz" : "PR: " + reader.pr + " bpm"
            font.pixelSize: 24
        }
    }
}
