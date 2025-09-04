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

            // AYAR BUTONU - Sağ üst köşe
            Button {
                id: settingsButton
                text: "⚙️ Ayarlar"
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 20
                onClicked: settingsPopup.open()
            }

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
                            ctx.strokeStyle = "#D3D3D3"
                            ctx.lineWidth = 0.5

                            var gridSpacingY = height / 8
                            for (var i = 1; i < 8; i++) {
                                ctx.beginPath()
                                ctx.moveTo(0, i * gridSpacingY)
                                ctx.lineTo(width, i * gridSpacingY)
                                ctx.stroke()
                            }

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

                            ctx.strokeStyle = "#006400"
                            ctx.lineWidth = 2
                            ctx.lineCap = "round"
                            ctx.lineJoin = "round"

                            ctx.beginPath()

                            var dataPoints = reader.waveform
                            var step = width / (dataPoints.length - 1)

                            if (dataPoints.length > 0) {
                                var y0 = height - (dataPoints[0] / 255.0 * height)
                                ctx.moveTo(0, y0)

                                for (var i = 1; i < dataPoints.length; i++) {
                                    var x = i * step
                                    var y = height - (dataPoints[i] / 255.0 * height)

                                    if (i === 1) {
                                        ctx.lineTo(x, y)
                                    } else {
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
                                ctx.strokeStyle = "rgba(0, 100, 0, 0.3)"
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
                        var patientName = ""

                        if (firstNameField.text.trim() !== "" && lastNameField.text.trim() !== "") {
                            patientName = firstNameField.text.trim() + " " + lastNameField.text.trim()
                        } else {
                            patientName = measurementModel.getLastPatientName()
                        }

                        var success = pdfExporter.exportWaveformToPdf(
                            waveformCanvas,
                            patientName,
                            reader.spo2,
                            reader.pr
                        )

                        if (success) {
                            statusText.text = "PDF başarıyla masaüstüne kaydedildi!"
                            statusText.color = "green"
                        } else {
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

                Timer {
                    id: statusTimer
                    interval: 3000
                    repeat: false
                    onTriggered: statusText.text = ""
                }

                // VERİ SAYFASINA GEÇ & FREEZE BUTONU
                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: "Veri Tablosunu Aç"
                        onClicked: stackView.push(dataPage)
                    }

                    Button {
                        text: reader.frozen ? "🔓 Devam Et" : "🔒 Freeze"
                        background: Rectangle {
                            color: reader.frozen ? "#4CAF50" : "#FF9800"
                            radius: 4
                            border.color: reader.frozen ? "#45a049" : "#f57c00"
                            border.width: 1
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            reader.toggleFreeze()
                            if (reader.frozen) {
                                console.log("❄️ Waveform donduruldu")
                            } else {
                                console.log("🔥 Waveform devam ediyor")
                            }
                        }
                    }
                }
            }
        }
    }

    // --- AYAR POPUP'I ---
    Popup {
        id: settingsPopup
        width: 300
        height: 200
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Rectangle {
            anchors.fill: parent
            color: "white"
            border.color: "gray"
            border.width: 1
            radius: 10

            Column {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    text: "SpO2 Response Time Ayarı"
                    font.pixelSize: 16
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        text: "Response Time:"; anchors.verticalCenter: parent.verticalCenter
                    }

                    ComboBox {
                        id: responseTimeCombo
                        width: 120
                        model: [
                            { text: "4 saniye", value: 4 },
                            { text: "8 saniye", value: 8 },
                            { text: "16 saniye", value: 16 }
                        ]
                        textRole: "text"
                        currentIndex: 1
                        onCurrentTextChanged: console.log("Response time seçildi:", currentText)
                    }
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        text: "Uygula"
                        onClicked: {
                            var selectedValue = responseTimeCombo.model[responseTimeCombo.currentIndex].value
                            if (reader.setResponseTime(selectedValue)) {
                                settingsStatusText.text = "Response time " + selectedValue + " saniye olarak ayarlandı"
                                settingsStatusText.color = "green"
                            } else {
                                settingsStatusText.text = "Ayar gönderilemedi!"
                                settingsStatusText.color = "red"
                            }
                            settingsStatusTimer.start()
                        }
                    }

                    Button { text: "Kapat"; onClicked: settingsPopup.close() }
                }

                Text {
                    id: settingsStatusText
                    text: ""
                    font.pixelSize: 12
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Timer {
                    id: settingsStatusTimer
                    interval: 3000
                    repeat: false
                    onTriggered: settingsStatusText.text = ""
                }
            }
        }
    }

    // --- VERİ SAYFASI ---
    Component {
        id: dataPage
        Page {
            title: "Tüm Kayıtlı Veriler"
            onVisibleChanged: if (visible) measurementModel.refreshData()

            Column {
                anchors.fill: parent
                spacing: 10
                anchors.margins: 10

                // FİLTRE ALANI
                Rectangle {
                    width: parent.width
                    height: 120
                    color: "#f0f0f0"
                    border.color: "gray"
                    border.width: 1
                    radius: 5

                    Column {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Text {
                            text: "📊 Filtre Ayarları"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Row {
                            spacing: 20
                            width: parent.width

                            Column {
                                spacing: 5
                                Text { text: "SpO₂ Aralığı:"; font.pixelSize: 12 }
                                Row {
                                    spacing: 5
                                    SpinBox { id: spo2MinSpin; from:0; to:100; value:0; editable:true; width:80 }
                                    Text { text: "-"; anchors.verticalCenter: parent.verticalCenter }
                                    SpinBox { id: spo2MaxSpin; from:0; to:100; value:0; editable:true; width:80 }
                                }
                            }

                            Column {
                                spacing: 5
                                Text { text: "PR Aralığı:"; font.pixelSize: 12 }
                                Row {
                                    spacing: 5
                                    SpinBox { id: prMinSpin; from:0; to:300; value:0; editable:true; width:80 }
                                    Text { text: "-"; anchors.verticalCenter: parent.verticalCenter }
                                    SpinBox { id: prMaxSpin; from:0; to:300; value:0; editable:true; width:80 }
                                }
                            }

                            Column {
                                spacing: 5
                                Text { text: " "; font.pixelSize: 12 }
                                Row {
                                    spacing: 10
                                    Button {
                                        text: "Filtrele"
                                        onClicked: measurementModel.applyFilter(spo2MinSpin.value, spo2MaxSpin.value, prMinSpin.value, prMaxSpin.value)
                                    }
                                    Button {
                                        text: "Temizle"
                                        onClicked: {
                                            spo2MinSpin.value=0; spo2MaxSpin.value=0
                                            prMinSpin.value=0; prMaxSpin.value=0
                                            measurementModel.clearFilter()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Row {
                    spacing: 80
                    Text { text: "Ad"; font.bold:true; width:80 }
                    Text { text: "Soyad"; font.bold:true; width:80 }
                    Text { text: "SpO₂"; font.bold:true; width:50 }
                    Text { text: "PR"; font.bold:true; width:50 }
                    Text { text: "Tarih/Saat"; font.bold:true; width:150 }
                }

                Rectangle { width: parent.width; height:1; color:"gray" }

                Text { id: emptyHint; visible: measurementModel.rowCount()===0; text:"Henüz kayıt yok."; color:"gray" }

                ListView {
                    id: dataListView
                    visible: measurementModel.rowCount()>0
                    width: parent.width
                    height: parent.height-280
                    model: measurementModel
                    clip:true

                    delegate: Row {
                        spacing: 80
                        Text { text: first_name || ""; width: 80 }
                        Text { text: last_name || ""; width: 80 }
                        Text { text: spo2 || ""; width: 50; color: spo2<90?"red":spo2<95?"orange":"black" }
                        Text { text: pr || ""; width: 50; color: pr<60||pr>100?"red":"black" }
                        Text { text: timestamp || ""; width: 150 }
                    }
                }

                Row {
                    spacing: 10
                    Button { text:"Yenile"; onClicked: measurementModel.refreshData() }
                    Button { text:"Geri"; onClicked: stackView.pop() }
                }
            }
        }
    }
}
