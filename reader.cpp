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

            // Timestamp'li buffer'a ekle
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            m_waveformBuffer.enqueue(WaveformPoint(smooth, currentTime));

            // Eski verileri temizle (20 saniyeden eski)
            cleanOldData();

            // Ekran için waveform güncelle
            updateDisplayWaveform();

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

void Reader::cleanOldData() {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 twentySecondsAgo = currentTime - (20 * 1000); // 20 saniye

    // 20 saniyeden eski verileri kaldır
    while (!m_waveformBuffer.isEmpty() &&
           m_waveformBuffer.first().timestamp < twentySecondsAgo) {
        m_waveformBuffer.dequeue();
    }
}

void Reader::updateDisplayWaveform() {
    // Ekran için son MAX_DISPLAY_POINTS kadar veriyi al
    m_waveform.clear();

    int totalPoints = m_waveformBuffer.size();
    int startIndex = qMax(0, totalPoints - MAX_DISPLAY_POINTS);

    for (int i = startIndex; i < totalPoints; ++i) {
        m_waveform.append(m_waveformBuffer.at(i).value);
    }
}

QVariantList Reader::getLast20SecondsWaveform() const {
    QVariantList result;

    for (const WaveformPoint &point : m_waveformBuffer) {
        result.append(point.value);
    }

    qDebug() << "getLast20SecondsWaveform() - dönen nokta sayısı:" << result.size();
    return result;
}

QVariantList Reader::getLast20SecondsTimestamps() const {
    QVariantList result;

    for (const WaveformPoint &point : m_waveformBuffer) {
        result.append(point.timestamp);
    }

    return result;
}
