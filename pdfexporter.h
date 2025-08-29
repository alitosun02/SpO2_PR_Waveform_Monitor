#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include <QObject>
#include <QQuickItem>
#include <QPainter>
#include <QPdfWriter>
#include <QQuickPaintedItem>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QQmlEngine>
#include <QVariantList>
#include <QPainterPath>
#include <QColor>

class PdfExporter : public QObject
{
    Q_OBJECT

public:
    explicit PdfExporter(QObject *parent = nullptr);

    Q_INVOKABLE bool exportWaveformToPdf(QQuickItem *canvas,
                                         const QString &patientName = "",
                                         int spo2Value = -1,
                                         int prValue = -1);

private:
    QString getDesktopPath() const;
    QString generateFileName(const QString &patientName) const;

    // Yeni metod: Waveform verilerini doğrudan çizmek için
    void drawWaveformData(QPainter &painter, const QRect &rect, const QVariantList &waveformData);
};

#endif // PDFEXPORTER_H
