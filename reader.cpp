#include "reader.h"
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

Reader::Reader(const QString &portName, QObject *parent)
    : QObject(parent)
{
    serial.setPortName(portName);
    serial.setBaudRate(375000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::OddParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        qCritical() << "Port açılamadı:" << serial.errorString();
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        return;
    }

    qDebug() << serial.write(QByteArray::fromHex("BF 5F FF"));

    connect(&serial, &QSerialPort::readyRead, this, &Reader::readSerialData);
    qDebug() << portName << "açıldı, veri bekleniyor...";
}

void Reader::readSerialData() {
    buffer.append(serial.readAll());

    while (true) {
        int start = buffer.indexOf(QByteArray::fromHex("AA55"));
        if (start < 0) { buffer.clear(); break; }
        if (start > 0) buffer.remove(0, start);

        if (buffer.size() < 3) break;
        quint8 len = static_cast<quint8>(buffer[2]);
        int totalSize = 2 + 1 + len + 1;
        if (buffer.size() < totalSize) break;

        QByteArray packet = buffer.left(totalSize);
        processPacket(packet);
        buffer.remove(0, totalSize);
    }
}

void Reader::processPacket(const QByteArray &packet) {
    quint8 len  = static_cast<quint8>(packet[2]);
    quint8 code = static_cast<quint8>(packet[3]);

    quint8 sum = len;
    for (int i = 3; i < 3 + len; ++i)
        sum += static_cast<quint8>(packet[i]);
    sum &= 0xFF;
    if (sum != static_cast<quint8>(packet[packet.size()-1])) return;

    if (code == 21 && len == 10) {
        quint8 spo2   = static_cast<quint8>(packet[7]);
        quint8 pr_msb = static_cast<quint8>(packet[8]);
        quint8 pr_lsb = static_cast<quint8>(packet[9]);
        int pr = (pr_msb << 8) | pr_lsb;

        if (spo2 == 127) {
            qDebug() << "SpO2: Geçersiz";
        } else {
            qDebug().nospace() << "SpO2: " << spo2 << " %";
        }

        if (pr == 255) {
            qDebug() << "PR: Geçersiz";
        } else {
            qDebug().nospace() << "PR: " << pr << " bpm";
        }
    }
}
