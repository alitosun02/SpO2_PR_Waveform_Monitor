import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: appWindow
    width: 800
    height: 600
    visible: true
    title: "SpO2, PR ve Waveform"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainPage
    }

    // --- ANA SAYFA ---
    Component {
        id: mainPage
        Page {
            title: "Ana Sayfa"
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
                            if (measurementModel.addPatient(firstNameField.text || "", lastNameField.text || "")) {
                                console.log("Hasta kaydedildi")
                                firstNameField.text = ""
                                lastNameField.text = ""
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
                                height: reader.spo2 === -1 ? 0 : (reader.spo2 / 100.0 * parent.height)
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
                                height: reader.pr === -1 ? 0 : (reader.pr / 200.0 * parent.height)
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

                // VERİ SAYFASINA GEÇ
                Button {
                    text: "Veri Tablosunu Aç"
                    onClicked: stackView.push(dataPage)
                }
            }
        }
    }

    // --- VERİ SAYFASI ---
    Component {
        id: dataPage
        Page {
            title: "Tüm Kayıtlı Veriler"

            // Sayfa görünür oldukça veriyi yenile
            onVisibleChanged: if (visible) measurementModel.refreshData()

            Column {
                anchors.fill: parent
                spacing: 10
                padding: 10

                // Başlık satırı
                Row {
                    spacing: 80
                    Text { text: "Ad"; font.bold: true; width: 80 }
                    Text { text: "Soyad"; font.bold: true; width: 80 }
                    Text { text: "SpO₂"; font.bold: true; width: 50 }
                    Text { text: "PR"; font.bold: true; width: 50 }
                    Text { text: "Tarih/Saat"; font.bold: true; width: 150 }
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: "gray"
                }

                // "Kayıt yok" mesajı
                Text {
                    id: emptyHint
                    visible: measurementModel.rowCount() === 0
                    text: "Henüz kayıt yok."
                    color: "gray"
                }

                // Liste
                ListView {
                    id: dataListView
                    visible: measurementModel.rowCount() > 0
                    width: parent.width
                    height: parent.height - 150
                    model: measurementModel
                    clip: true

                    delegate: Row {
                        spacing: 80
                        Text { text: first_name || ""; width: 80 }
                        Text { text: last_name || ""; width: 80 }
                        Text { text: spo2 || ""; width: 50 }
                        Text { text: pr || ""; width: 50 }
                        Text { text: timestamp || ""; width: 150 }
                    }
                }

                Row {
                    spacing: 10
                    Button { text: "Yenile"; onClicked: measurementModel.refreshData() }
                    Button { text: "Geri"; onClicked: stackView.pop() }
                }
            }
        }
    }
}
