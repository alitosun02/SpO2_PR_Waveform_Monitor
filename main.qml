import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "SpO2, PR ve Waveform"

    Column {
        anchors.centerIn: parent
        spacing: 20

        // üstte yan yana barlar
        Row {
            spacing: 20

            // SPO2 yazı
                  Text {
                      id: spo2Text
                      text: reader.spo2 === -1 ? "SpO2: -" : "SpO2: " + reader.spo2 + " %"
                      font.pixelSize: 20
                  }

            Rectangle {
                width: 100; height: 150; color: "lightgray"
                Rectangle {
                    width: parent.width
                    height: reader.spo2
                    anchors.bottom: parent.bottom
                    color: "red"
                }
            }

            Text {
                      id: prText
                      text: reader.pr === -1 ? "PR: -" : "PR: " + reader.pr + " bpm"
                      font.pixelSize: 20
                  }

            Rectangle {
                width: 100; height: 150; color: "lightgray"
                Rectangle {
                    width: parent.width
                    height: reader.pr
                    anchors.bottom: parent.bottom
                    color: "blue"
                }
            }
        }

        Text {
                  id: wfText
                  text: "Waveform: "
                  font.pixelSize: 20
              }

        // altta çizgi grafiği
        Canvas {
            id: waveformCanvas
            width: 500; height: 150

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0,0,width,height)
                ctx.strokeStyle = "green"
                ctx.lineWidth = 2
                ctx.beginPath()

                if (reader.waveform.length > 0) {
                    var step = width / reader.waveform.length
                    ctx.moveTo(0, height - reader.waveform[0])
                    for (var i=1; i<reader.waveform.length; i++) {
                        ctx.lineTo(i*step, height - reader.waveform[i])
                    }
                }
                ctx.stroke()
            }

            Connections {
                target: reader
                function onWaveformChanged() { waveformCanvas.requestPaint() }
            }
        }
    }
}
