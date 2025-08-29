#include "pdfexporter.h"
#include <QQuickWindow>
#include <QOpenGLFramebufferObject>
#include <QQuickRenderControl>
#include <QApplication>
#include <QPageLayout>
#include <QPageSize>
#include <QQmlContext>

PdfExporter::PdfExporter(QObject *parent)
    : QObject(parent)
{
}

bool PdfExporter::exportWaveformToPdf(QQuickItem *canvas, const QString &patientName, int spo2Value, int prValue)
{
    if (!canvas) {
        qWarning() << "Canvas nesnesi bulunamadı!";
        return false;
    }

    try {
        QString actualPatientName = patientName.trimmed();
        qDebug() << "PDF için gelen hasta adı:" << actualPatientName;

        QString fileName = generateFileName(actualPatientName);
        QString fullPath = getDesktopPath() + "/" + fileName;

        QPdfWriter pdfWriter(fullPath);
        pdfWriter.setPageSize(QPageSize::A4);
        pdfWriter.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
        pdfWriter.setResolution(300);

        QPainter painter(&pdfWriter);

        // Basit koordinat sistemi - sadece painter.viewport() kullan
        QRect viewport = painter.viewport();
        int centerX = viewport.center().x();  // Tam orta nokta
        int pageWidth = viewport.width();
        int pageLeft = viewport.left();

        qDebug() << "Viewport:" << viewport;
        qDebug() << "Center X:" << centerX;

        // Font ayarları
        QFont titleFont("Arial", 18, QFont::Bold);
        QFont dateFont("Arial", 10);
        QFont sectionFont("Arial", 12, QFont::Bold);
        QFont normalFont("Arial", 10);

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        int currentY = viewport.top() + 80;
        const int spacing = 60;

        // BAŞLIK - Center noktasından başlayarak
        painter.setFont(titleFont);
        painter.setPen(Qt::black);

        QString title = "SpO₂ PR Waveform Raporu";
        QRect titleBounds = painter.fontMetrics().boundingRect(title);
        int titleStartX = centerX - titleBounds.width() / 2;

        painter.drawText(titleStartX, currentY, title);
        currentY += spacing;

        // TARİH
        painter.setFont(dateFont);
        QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
        QString dateText = "Tarih: " + dateTime;

        QRect dateBounds = painter.fontMetrics().boundingRect(dateText);
        int dateStartX = centerX - dateBounds.width() / 2;

        painter.drawText(dateStartX, currentY, dateText);
        currentY += spacing;

        // HASTA ADI
        painter.setFont(sectionFont);
        QString patientText = "Hasta: " + actualPatientName;

        QRect patientBounds = painter.fontMetrics().boundingRect(patientText);
        int patientStartX = centerX - patientBounds.width() / 2;

        painter.drawText(patientStartX, currentY, patientText);
        currentY += spacing;

        // AYIRAÇ ÇİZGİSİ - Ortadan eşit mesafede
        painter.setPen(QPen(Qt::black, 2));
        int lineLength = pageWidth / 3;
        int lineStartX = centerX - lineLength / 2;
        int lineEndX = centerX + lineLength / 2;
        painter.drawLine(lineStartX, currentY, lineEndX, currentY);
        currentY += spacing;

        // ÖLÇÜM SONUÇLARI BAŞLIĞI
        painter.setFont(sectionFont);
        painter.setPen(Qt::black);
        QString measureTitle = "Ölçüm Sonuçları";

        QRect measureTitleBounds = painter.fontMetrics().boundingRect(measureTitle);
        int measureTitleStartX = centerX - measureTitleBounds.width() / 2;

        painter.drawText(measureTitleStartX, currentY, measureTitle);
        currentY += spacing;

        // ÖLÇÜM DEĞERLERİ
        painter.setFont(normalFont);

        if (spo2Value != -1 && prValue != -1) {
            QString measurementText = QString("SpO₂: %1 %    •    Nabız: %2 bpm").arg(spo2Value).arg(prValue);

            QRect measureBounds = painter.fontMetrics().boundingRect(measurementText);
            int measureStartX = centerX - measureBounds.width() / 2;

            painter.drawText(measureStartX, currentY, measurementText);
        }
        currentY += spacing;

        // WAVEFORM BAŞLIĞI
        painter.setFont(sectionFont);
        QString waveTitle = "Son 20 Saniye Waveform Grafiği";

        QRect waveTitleBounds = painter.fontMetrics().boundingRect(waveTitle);
        int waveTitleStartX = centerX - waveTitleBounds.width() / 2;

        painter.drawText(waveTitleStartX, currentY, waveTitle);
        currentY += spacing;

        // *** ANA DEĞIŞIKLIK: Reader'dan 20 saniyelik veriyi al ve çiz ***
        // Canvas'tan görüntü almak yerine, doğrudan Reader'dan veri alıp çiziyoruz

        // QML context'inden reader nesnesine erişim
        QObject *readerObj = nullptr;
        if (canvas->window()) {
            QQmlEngine *engine = qmlEngine(canvas);
            if (engine) {
                QQmlContext *context = engine->rootContext();
                if (context) {
                    readerObj = context->contextProperty("reader").value<QObject*>();
                }
            }
        }

        if (readerObj) {
            // Reader'dan son 20 saniyelik waveform verisini al
            QVariantList waveformData;
            QMetaObject::invokeMethod(readerObj, "getLast20SecondsWaveform",
                                      Q_RETURN_ARG(QVariantList, waveformData));

            qDebug() << "PDF için alınan waveform nokta sayısı:" << waveformData.size();

            if (!waveformData.isEmpty()) {
                // Grafik boyutları - sayfa genişliğinin %75'i
                int imageWidth = static_cast<int>(pageWidth * 0.75);
                int imageHeight = 280;

                // Sayfaya sığar mı kontrol
                int remainingHeight = viewport.bottom() - currentY - 80;
                if (imageHeight > remainingHeight) {
                    imageHeight = remainingHeight;
                }

                // TAM ORTALA - centerX'ten başlayarak
                int imageStartX = centerX - imageWidth / 2;
                QRect targetRect(imageStartX, currentY, imageWidth, imageHeight);

                // Bej arka plan
                painter.fillRect(targetRect, QColor("#F5F5DC"));

                // Waveform verilerini çiz
                drawWaveformData(painter, targetRect, waveformData);

                // Çerçeve
                painter.setPen(QPen(Qt::black, 2));
                painter.drawRect(targetRect);

                // Alt not - daha fazla boşluk bırak
                currentY = targetRect.bottom() + 100;
                painter.setFont(QFont("Arial", 8));
                painter.setPen(Qt::darkGray);

                QString note = "* Grafikte son 20 saniyeye ait waveform verileri gösterilmektedir";
                QRect noteBounds = painter.fontMetrics().boundingRect(note);
                int noteStartX = centerX - noteBounds.width() / 2;

                painter.drawText(noteStartX, currentY, note);

                qDebug() << "PDF başarıyla oluşturuldu:" << fullPath;
                qDebug() << "Waveform verileri çizildi. Nokta sayısı:" << waveformData.size();
                return true;
            } else {
                // Veri yok mesajı
                painter.setFont(normalFont);
                painter.setPen(Qt::red);
                QString errorMsg = "20 saniyelik waveform verisi henüz biriktirilmedi";

                QRect errorBounds = painter.fontMetrics().boundingRect(errorMsg);
                int errorStartX = centerX - errorBounds.width() / 2;

                painter.drawText(errorStartX, currentY, errorMsg);
                return true;
            }
        } else {
            // Reader nesnesine erişilemedi
            painter.setFont(normalFont);
            painter.setPen(Qt::red);
            QString errorMsg = "Waveform verisine erişilemiyor";

            QRect errorBounds = painter.fontMetrics().boundingRect(errorMsg);
            int errorStartX = centerX - errorBounds.width() / 2;

            painter.drawText(errorStartX, currentY, errorMsg);
            return true;
        }

    } catch (const std::exception &e) {
        qCritical() << "PDF oluşturulurken hata:" << e.what();
        return false;
    }

    return false;
}

void PdfExporter::drawWaveformData(QPainter &painter, const QRect &rect, const QVariantList &waveformData)
{
    if (waveformData.isEmpty()) return;

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor("#006400"), 2)); // Koyu yeşil (Dark Green)

    int dataCount = waveformData.size();
    if (dataCount < 2) return;

    // X ekseni adımları
    double stepX = static_cast<double>(rect.width()) / (dataCount - 1);

    // Y ekseni normalizasyonu (0-255 değerlerini rect yüksekliğine sığdır)
    int rectTop = rect.top();
    int rectBottom = rect.bottom();
    int rectHeight = rect.height();

    QPainterPath path;
    bool firstPoint = true;

    for (int i = 0; i < dataCount; ++i) {
        bool ok;
        double value = waveformData.at(i).toDouble(&ok);
        if (!ok) continue;

        // X koordinatı
        double x = rect.left() + (i * stepX);

        // Y koordinatı (değeri rect içine sığdır, ters çevir çünkü PDF'de Y yukarı doğru artar)
        double normalizedValue = qBound(0.0, value / 255.0, 1.0); // 0-1 arasına normalize et
        double y = rectBottom - (normalizedValue * rectHeight);

        if (firstPoint) {
            path.moveTo(x, y);
            firstPoint = false;
        } else {
            path.lineTo(x, y);
        }
    }

    painter.drawPath(path);

    // Zaman ekseninde işaretlemeler ekle
    painter.setPen(QPen(Qt::gray, 1));
    QFont timeFont("Arial", 8);
    painter.setFont(timeFont);

    // 5 saniye aralıklarla işaretleme
    for (int seconds = 0; seconds <= 20; seconds += 5) {
        double progress = static_cast<double>(seconds) / 20.0;
        double x = rect.left() + (progress * rect.width());

        // Dikey çizgi
        painter.drawLine(x, rect.bottom(), x, rect.bottom() + 5);

        // Zaman etiketi
        QString timeLabel = QString("-%1s").arg(20 - seconds);
        QRect textRect = painter.fontMetrics().boundingRect(timeLabel);
        painter.drawText(x - textRect.width()/2, rect.bottom() + 35, timeLabel);
    }
}

QString PdfExporter::getDesktopPath() const
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty()) {
        desktopPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QDir dir(desktopPath);
    if (!dir.exists()) {
        dir.mkpath(desktopPath);
    }

    return desktopPath;
}

QString PdfExporter::generateFileName(const QString &patientName) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName;

    if (!patientName.isEmpty() && patientName != "Kayıt Yok" && patientName != "Hasta Bulunamadı") {
        QString cleanName = patientName;
        cleanName = cleanName.replace("ç", "c").replace("Ç", "C")
                        .replace("ğ", "g").replace("Ğ", "G")
                        .replace("ı", "i").replace("İ", "I")
                        .replace("ö", "o").replace("Ö", "O")
                        .replace("ş", "s").replace("Ş", "S")
                        .replace("ü", "u").replace("Ü", "U")
                        .replace(" ", "_");

        fileName = QString("Waveform_%1_%2.pdf").arg(cleanName).arg(timestamp);
    } else {
        fileName = QString("Waveform_%1.pdf").arg(timestamp);
    }

    return fileName;
}
