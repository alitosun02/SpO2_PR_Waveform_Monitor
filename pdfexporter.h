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
};

#endif // PDFEXPORTER_H
