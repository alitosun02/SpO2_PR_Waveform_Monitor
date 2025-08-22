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

        // HASTA EKLEME FORMU
        Row {
            spacing: 10
            TextField { id: firstNameField; placeholderText: "Ad" }
            TextField { id: lastNameField; placeholderText: "Soyad" }
            Button {
                text: "Hasta Kaydet"
                onClicked: {
                    console.log("Ad:", firstNameField.text, "Soyad:", lastNameField.text)
                    if (dbManager.addPatient(firstNameField.text || "", lastNameField.text || "")) {
                        console.log("Hasta kaydedildi")
                    } else {
                        console.log("Hasta kaydı başarısız!")
                    }
                }
            }
        }

        // SPO2 & PR BARLARI
        Row {
            spacing: 20

            Column {
                Text { text: reader.spo2 === -1 ? "SpO2: -" : "SpO2: " + reader.spo2 + " %" }
                Rectangle {
                    width: 100; height: 150; color: "lightgray"
                    Rectangle {
                        width: parent.width
                        height: reader.spo2
                        anchors.bottom: parent.bottom
                        color: "red"
                    }
                }
            }

            Column {
                Text { text: reader.pr === -1 ? "PR: -" : "PR: " + reader.pr + " bpm" }
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
        }

        // WAVEFORM
        Text { text: "Waveform:"; font.pixelSize: 20 }
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
