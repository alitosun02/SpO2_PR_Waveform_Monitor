#include "pdfexporter.h"
#include <QQuickWindow>
#include <QOpenGLFramebufferObject>
#include <QQuickRenderControl>
#include <QApplication>
#include <QPageLayout>
#include <QPageSize>

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

        // WAVEFORM GRAFİĞİ - Tam ortala
        QQuickWindow *window = canvas->window();
        if (window) {
            QPointF canvasPos = canvas->mapToScene(QPointF(0, 0));
            QSizeF canvasSize = QSizeF(canvas->width(), canvas->height());

            QImage windowImage = window->grabWindow();

            if (!windowImage.isNull()) {
                QRect canvasRect(
                    qBound(0, static_cast<int>(canvasPos.x()), windowImage.width()),
                    qBound(0, static_cast<int>(canvasPos.y()), windowImage.height()),
                    qMin(static_cast<int>(canvasSize.width()), windowImage.width() - static_cast<int>(canvasPos.x())),
                    qMin(static_cast<int>(canvasSize.height()), windowImage.height() - static_cast<int>(canvasPos.y()))
                    );

                QImage canvasImage = windowImage.copy(canvasRect);

                if (!canvasImage.isNull() && canvasImage.width() > 0 && canvasImage.height() > 0) {
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

                    // Beyaz arka plan
                    painter.fillRect(targetRect, Qt::white);

                    // Grafik çiz
                    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                    painter.drawImage(targetRect, canvasImage);

                    // Çerçeve
                    painter.setPen(QPen(Qt::black, 2));
                    painter.drawRect(targetRect);

                    // Alt not
                    currentY = targetRect.bottom() + 25;
                    painter.setFont(QFont("Arial", 8));
                    painter.setPen(Qt::darkGray);

                    QString note = "* Grafikte son 20 saniyeye ait waveform verileri gösterilmektedir";
                    QRect noteBounds = painter.fontMetrics().boundingRect(note);
                    int noteStartX = centerX - noteBounds.width() / 2;

                    painter.drawText(noteStartX, currentY, note);

                    qDebug() << "PDF başarıyla oluşturuldu:" << fullPath;
                    qDebug() << "Grafik tam ortada - X:" << imageStartX << "Center:" << centerX;
                    return true;
                } else {
                    // Hata mesajı da ortalı
                    painter.setFont(normalFont);
                    painter.setPen(Qt::red);
                    QString errorMsg = "Waveform verisi bulunamadı";

                    QRect errorBounds = painter.fontMetrics().boundingRect(errorMsg);
                    int errorStartX = centerX - errorBounds.width() / 2;

                    painter.drawText(errorStartX, currentY, errorMsg);
                    return true;
                }
            }
        }
        return false;

    } catch (const std::exception &e) {
        qCritical() << "PDF oluşturulurken hata:" << e.what();
        return false;
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
