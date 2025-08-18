#ifndef READER_H
#define READER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class Reader : public QObject
{
    Q_OBJECT
public:
    explicit Reader(const QString &portName, QObject *parent = nullptr);

private slots:
    void readSerialData();
    void processPacket(const QByteArray &packet);

private:
    QSerialPort serial;
    QByteArray buffer;
};

#endif // READER_H
