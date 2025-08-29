#include "reader.h"
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <QDateTime>

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

    // Başlangıç komutu
    serial.write(QByteArray::fromHex("BF5FFF"));
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
    if (packet.size() < 5) return;

    quint8 len  = static_cast<quint8>(packet[2]);
    quint8 code = static_cast<quint8>(packet[3]);

    // checksum hesapla
    quint8 sum = len;
    for (int i = 3; i < 3 + len && i < packet.size(); ++i)
        sum += static_cast<quint8>(packet[i]);
    sum &= 0xFF;

    if (sum != static_cast<quint8>(packet[packet.size()-1])) return;

    if (code == 21 && len >= 10) {
        // waveform
        quint8 waveformVal = static_cast<quint8>(packet[5]);
        if (waveformVal != 127) {
            static int lastValue = 0;
            int smooth = (lastValue + waveformVal) / 2; // basit ortalama
            lastValue = smooth;

            m_waveform.append(smooth);

            // 20 saniye için maksimum 1000 nokta tutuyoruz (yaklaşık 50Hz sampling)
            // Bu daha gerçekçi ve performanslı
            if (m_waveform.size() > 1000)
                m_waveform.removeFirst();

            emit waveformChanged();
        }

        // SPO2
        quint8 spo2  = static_cast<quint8>(packet[7]);
        if (spo2 == 127) {
            m_spo2 = -1;
        } else {
            m_spo2 = spo2;
        }
        emit spo2Changed();

        // PR
        quint8 pr_msb = static_cast<quint8>(packet[8]);
        quint8 pr_lsb = static_cast<quint8>(packet[9]);
        int pr = (pr_msb << 8) | pr_lsb;
        if (pr == 255) {
            m_pr = -1;
        } else {
            m_pr = pr;
        }
        emit prChanged();
    }
}

QVariantList Reader::getLast20SecondsWaveform() const {
    // Tüm mevcut veriyi döndür (zaten maksimum 20 saniye tutuluyor)
    QVariantList result;
    for (const QVariant &point : m_waveform) {
        result.append(point);
    }

    qDebug() << "getLast20SecondsWaveform() - dönen nokta sayısı:" << result.size();
    return result;
}
