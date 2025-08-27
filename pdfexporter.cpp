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
        // PDF dosyasının kaydedileceği yol
        QString fileName = generateFileName(patientName);
        QString fullPath = getDesktopPath() + "/" + fileName;

        qDebug() << "PDF kaydedilecek yer:" << fullPath;

        // PDF Writer oluştur
        QPdfWriter pdfWriter(fullPath);
        pdfWriter.setPageSize(QPageSize::A4);
        pdfWriter.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
        pdfWriter.setResolution(300); // Yüksek çözünürlük

        QPainter painter(&pdfWriter);

        // Sayfa boyutlarını al (pixel cinsinden)
        QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

        // Türkçe karakter desteği için font ayarla
        QFont titleFont("Arial", 18, QFont::Bold);
        QFont normalFont("Arial", 12);
        QFont boldFont("Arial", 12, QFont::Bold);

        // Y pozisyonu takibi için
        int currentY = 100;
        const int lineSpacing = 80;
        const int sectionSpacing = 120;

        // BAŞLIK
        painter.setFont(titleFont);
        painter.setPen(Qt::black);
        QRect titleRect(pageRect.left(), currentY, pageRect.width(), lineSpacing);
        painter.drawText(titleRect, Qt::AlignCenter, "SpO₂ PR Waveform Raporu");
        currentY += sectionSpacing;

        // TARİH VE SAAT - Sağ üstte
        painter.setFont(normalFont);
        QString dateTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
        QRect dateRect(pageRect.right() - 400, currentY, 400, lineSpacing);
        painter.drawText(dateRect, Qt::AlignRight, "Tarih: " + dateTime);
        currentY += sectionSpacing;

        // HASTA BİLGİSİ
        if (!patientName.isEmpty()) {
            painter.setFont(boldFont);
            QRect patientRect(pageRect.left(), currentY, pageRect.width(), lineSpacing);
            painter.drawText(patientRect, Qt::AlignLeft, "Hasta: " + patientName);
            currentY += lineSpacing;
        }

        // ÖLÇÜM DEĞERLERİ
        painter.setFont(boldFont);
        painter.drawText(pageRect.left(), currentY, "Ölçüm Sonuçları:");
        currentY += lineSpacing;

        painter.setFont(normalFont);

        if (spo2Value != -1) {
            QString spo2Text = QString("SpO₂: %1 %").arg(spo2Value);
            painter.drawText(pageRect.left() + 50, currentY, spo2Text);
            currentY += lineSpacing;
        }

        if (prValue != -1) {
            QString prText = QString("Nabız: %1 bpm").arg(prValue);
            painter.drawText(pageRect.left() + 50, currentY, prText);
            currentY += lineSpacing;
        }

        currentY += sectionSpacing;

        // WAVEFORM BAŞLIĞI
        painter.setFont(boldFont);
        painter.drawText(pageRect.left(), currentY, "Waveform Grafiği:");
        currentY += lineSpacing + 50;

        // CANVAS RESMİNİ YAKALA VE EKLE
        QQuickWindow *window = canvas->window();
        if (window) {
            // Canvas'ın konumunu ve boyutunu al
            QPointF canvasPos = canvas->mapToScene(QPointF(0, 0));
            QSizeF canvasSize = QSizeF(canvas->width(), canvas->height());

            // Tüm pencereyi yakala
            QImage windowImage = window->grabWindow();

            if (!windowImage.isNull()) {
                // Canvas kısmını kırp
                QRect canvasRect(
                    static_cast<int>(canvasPos.x()),
                    static_cast<int>(canvasPos.y()),
                    static_cast<int>(canvasSize.width()),
                    static_cast<int>(canvasSize.height())
                    );

                // Güvenli kırpma - sınırları kontrol et
                canvasRect = canvasRect.intersected(windowImage.rect());
                QImage canvasImage = windowImage.copy(canvasRect);

                if (!canvasImage.isNull()) {
                    // Canvas resmini daha büyük boyutlarda PDF'e yerleştir
                    int imageWidth = pageRect.width() - 100; // Kenar boşlukları
                    int imageHeight = static_cast<int>(imageWidth * 0.3); // En boy oranı

                    // Sayfaya sığacak boyutta olup olmadığını kontrol et
                    int remainingHeight = pageRect.bottom() - currentY - 100;
                    if (imageHeight > remainingHeight) {
                        imageHeight = remainingHeight;
                        imageWidth = static_cast<int>(imageHeight / 0.3);
                    }

                    QRect targetRect(
                        pageRect.left() + 50,
                        currentY,
                        imageWidth,
                        imageHeight
                        );

                    // Yüksek kaliteli render için smooth transformation
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

                    painter.drawImage(targetRect, canvasImage);

                    // Waveform etrafına çerçeve çiz
                    painter.setPen(QPen(Qt::black, 2));
                    painter.drawRect(targetRect);

                    qDebug() << "PDF başarıyla oluşturuldu:" << fullPath;
                    qDebug() << "Canvas boyutu:" << canvasSize;
                    qDebug() << "PDF'deki görüntü boyutu:" << targetRect.size();

                    return true;
                } else {
                    qWarning() << "Canvas resmi kırpılamadı!";
                    return false;
                }
            } else {
                qWarning() << "Pencere görüntüsü alınamadı!";
                return false;
            }
        } else {
            qWarning() << "Canvas window bulunamadı!";
            return false;
        }

    } catch (const std::exception &e) {
        qCritical() << "PDF oluşturulurken hata:" << e.what();
        return false;
    }
}

QString PdfExporter::getDesktopPath() const
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (desktopPath.isEmpty()) {
        // Desktop path bulunamazsa Documents kullan
        desktopPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    // Klasör yoksa oluştur
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

    if (!patientName.isEmpty()) {
        // Hasta adını dosya adına uygun hale getir (Türkçe karakterleri değiştir)
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
