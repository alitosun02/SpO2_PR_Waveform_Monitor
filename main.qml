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

                // WAVEFORM - Gelişmiş versiyon
                                Text { text: "Waveform:"; font.pixelSize: 20 }

                                Rectangle {
                                    width: 520; height: 170
                                    color: "#F5F5DC" // Bej arka plan
                                    border.color: "black"
                                    border.width: 2

                                    Canvas {
                                        id: waveformCanvas
                                        anchors.fill: parent
                                        anchors.margins: 10

                                        onPaint: {
                                            var ctx = getContext("2d")
                                            ctx.clearRect(0, 0, width, height)

                                            // Grid çizgilerini çiz
                                            drawGrid(ctx)

                                            // Waveform çiz
                                            drawWaveform(ctx)
                                        }

                                        function drawGrid(ctx) {
                                            ctx.strokeStyle = "#D3D3D3" // Açık gri
                                            ctx.lineWidth = 0.5

                                            // Yatay grid çizgileri
                                            var gridSpacingY = height / 8
                                            for (var i = 1; i < 8; i++) {
                                                ctx.beginPath()
                                                ctx.moveTo(0, i * gridSpacingY)
                                                ctx.lineTo(width, i * gridSpacingY)
                                                ctx.stroke()
                                            }

                                            // Dikey grid çizgileri
                                            var gridSpacingX = width / 10
                                            for (var i = 1; i < 10; i++) {
                                                ctx.beginPath()
                                                ctx.moveTo(i * gridSpacingX, 0)
                                                ctx.lineTo(i * gridSpacingX, height)
                                                ctx.stroke()
                                            }
                                        }

                                        function drawWaveform(ctx) {
                                            if (reader.waveform.length === 0) return

                                            ctx.strokeStyle = "#006400" // Koyu yeşil
                                            ctx.lineWidth = 2
                                            ctx.lineCap = "round"
                                            ctx.lineJoin = "round"

                                            // Smooth path çizimi
                                            ctx.beginPath()

                                            var dataPoints = reader.waveform
                                            var step = width / (dataPoints.length - 1)

                                            // İlk nokta
                                            if (dataPoints.length > 0) {
                                                var y0 = height - (dataPoints[0] / 255.0 * height)
                                                ctx.moveTo(0, y0)

                                                // Bezier curve kullanarak smooth çizgi
                                                for (var i = 1; i < dataPoints.length; i++) {
                                                    var x = i * step
                                                    var y = height - (dataPoints[i] / 255.0 * height)

                                                    if (i === 1) {
                                                        ctx.lineTo(x, y)
                                                    } else {
                                                        // Smooth curve için control points
                                                        var prevX = (i-1) * step
                                                        var prevY = height - (dataPoints[i-1] / 255.0 * height)

                                                        var cpX1 = prevX + step * 0.3
                                                        var cpY1 = prevY
                                                        var cpX2 = x - step * 0.3
                                                        var cpY2 = y

                                                        ctx.bezierCurveTo(cpX1, cpY1, cpX2, cpY2, x, y)
                                                    }
                                                }

                                                ctx.stroke()

                                                // Gölge efekti
                                                ctx.strokeStyle = "rgba(0, 100, 0, 0.3)" // Şeffaf koyu yeşil
                                                ctx.lineWidth = 4
                                                ctx.stroke()
                                            }
                                        }

                                        Connections {
                                            target: reader
                                            function onWaveformChanged() {
                                                waveformCanvas.requestPaint()
                                            }
                                        }
                                    }

                                    // Zaman ekseni etiketleri
                                    Row {
                                        anchors.bottom: parent.bottom
                                        anchors.bottomMargin: -25
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.leftMargin: 10
                                        anchors.rightMargin: 10

                                        Repeater {
                                            model: ["Now", "-2s", "-4s", "-6s", "-8s", "-10s"]
                                            Text {
                                                width: (parent.width / 6)
                                                text: modelData
                                                font.pixelSize: 10
                                                color: "#666666"
                                                horizontalAlignment: index === 0 ? Text.AlignRight : Text.AlignLeft
                                            }
                                        }
                                    }
                                }

                // PDF EXPORT BUTONU
                Button {
                    text: "Waveform'u PDF Olarak Kaydet"
                    enabled: measurementModel.hasActivePatient

                    onClicked: {
                        console.log("PDF buton tıklandı!")

                        // Önce text field'lardaki güncel değerleri kontrol et
                        var patientName = ""

                        if (firstNameField.text.trim() !== "" && lastNameField.text.trim() !== "") {
                            // Text field'larda değer varsa bunu kullan
                            patientName = firstNameField.text.trim() + " " + lastNameField.text.trim()
                            console.log("Text field'lardan alınan hasta adı:", patientName)
                        } else {
                            // Text field'lar boşsa model'den al
                            patientName = measurementModel.getLastPatientName()
                            console.log("Model'den alınan hasta adı:", patientName)
                        }

                        // PDF olarak kaydet
                        var success = pdfExporter.exportWaveformToPdf(
                            waveformCanvas,
                            patientName,
                            reader.spo2,
                            reader.pr
                        )

                        if (success) {
                            console.log("PDF başarıyla kaydedildi!")
                            statusText.text = "PDF başarıyla masaüstüne kaydedildi!"
                            statusText.color = "green"
                        } else {
                            console.log("PDF kaydedilemedi!")
                            statusText.text = "PDF kaydedilirken hata oluştu!"
                            statusText.color = "red"
                        }

                        statusTimer.start()
                    }
                }

                // DURUM MESAJI
                Text {
                    id: statusText
                    text: ""
                    font.pixelSize: 14
                    color: "green"
                }

                // DURUM MESAJINI TEMİZLEME TIMER'I
                Timer {
                    id: statusTimer
                    interval: 3000
                    repeat: false
                    onTriggered: statusText.text = ""
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
