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

                // --- YENİ BUTON ---
                Button {
                    text: "Veri Tablosunu Aç"
                    onClicked: stackView.push(dataPage)
                }
            }
        }
    }

    // --- 2 DAKİKALIK VERİ SAYFASI ---
    Component {
        id: dataPage
        Page {
            title: "Son 2 Dakika Verileri"

            Column {
                anchors.fill: parent
                spacing: 10
                padding: 10

                // Başlık satırı
                Row {
                    spacing: 20
                    Text { text: "Ad"; font.bold: true }
                    Text { text: "Soyad"; font.bold: true }
                    Text { text: "SpO₂"; font.bold: true }
                    Text { text: "PR"; font.bold: true }
                    Text { text: "Tarih/Saat"; font.bold: true }
                }

                // Liste
                ListView {
                    id: dataListView
                    model: dataListModel
                    clip: true
                    height: parent.height - 100
                    delegate: Row {
                        spacing: 20
                        Text { text: first_name }
                        Text { text: last_name }
                        Text { text: spo2 }
                        Text { text: pr }
                        Text { text: timestamp }
                    }
                }

                // Geri butonu
                Button {
                    text: "Geri"
                    onClicked: stackView.pop()
                }
            }

            ListModel { id: dataListModel }

            Component.onCompleted: {
                dataListModel.clear();
                var data = dbManager.getRecentData();
                for (var i = 0; i < data.length; ++i) {
                    dataListModel.append({
                        first_name: data[i].first_name,
                        last_name:  data[i].last_name,
                        spo2:       data[i].spo2,
                        pr:         data[i].pr,
                        timestamp:  data[i].timestamp
                    })
                }
            }
        }
    }
}
