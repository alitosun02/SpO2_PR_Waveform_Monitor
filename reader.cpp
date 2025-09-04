#include "reader.h"
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

Reader::Reader(const QString &portName, QObject *parent)
    : QObject(parent),
    m_portName(portName)
{
    // Seri port ayarları (ayrıntılar header'da bırakıldı)
    serial.setPortName(m_portName);
    serial.setBaudRate(375000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::OddParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!openSerialPort()) {
        qCritical() << "Port açılamadı:" << serial.errorString();
        // Uygulamayı kapatmayalım; sadece hata loglayalım.
        // Eğer kapatmak isterseniz: QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    } else {
        qDebug() << m_portName << "açıldı, veri bekleniyor...";
        // Başlangıç komutu (cihazın protokolüne göre)
        serial.write(QByteArray::fromHex("BF5FFF"));
        serial.flush();
    }
}

Reader::~Reader() {
    closeSerialPort();
}

bool Reader::openSerialPort() {
    if (serial.isOpen()) return true;
    serial.setPortName(m_portName);
    if (!serial.open(QIODevice::ReadWrite)) {
        qWarning() << "openSerialPort(): açılamadı:" << serial.errorString();
        return false;
    }
    // readyRead bağla (UniqueConnection ile tekrar bağlanmasını önle)
    connect(&serial, &QSerialPort::readyRead, this, &Reader::readSerialData, Qt::UniqueConnection);
    return true;
}

void Reader::closeSerialPort() {
    if (serial.isOpen()) {
        // Veri tamponlarını temizle
        serial.clear(QSerialPort::Input);
        serial.clear(QSerialPort::Output);
        disconnect(&serial, &QSerialPort::readyRead, this, &Reader::readSerialData);
        serial.close();
    }
}

void Reader::readSerialData() {
    // Eğer freeze durumundaysa normalde readyRead gelmeyecek çünkü port kapalı.
    // Ancak ek güvenlik olarak burada da kontrol edelim:
    if (m_frozen || !serial.isOpen()) {
        // Port kapalıysa ya da frozen ise hiçbir işlem yapma.
        return;
    }

    QByteArray incoming = serial.readAll();
    if (incoming.isEmpty()) return;

    buffer.append(incoming);

    // Paket işlemi: AA55 LEN CODE ... CHECKSUM
    while (true) {
        // Header arama
        int start = -1;
        for (int i = 0; i + 1 < buffer.size(); ++i) {
            if (static_cast<unsigned char>(buffer.at(i)) == 0xAA &&
                static_cast<unsigned char>(buffer.at(i+1)) == 0x55) {
                start = i;
                break;
            }
        }

        if (start < 0) {
            // Header yok — bekle, ancak buffer çok büyükse temizle
            if (buffer.size() > 4096) {
                buffer.clear();
                qWarning() << "readSerialData: header bulunamadı, buffer temizlendi (çok büyük).";
            }
            break;
        }

        if (start > 0) {
            // Başlangıç dışındaki ön veriyi at
            buffer.remove(0, start);
        }

        if (buffer.size() < 4) {
            // Başlık var ama yeterli veri yok (AA55 + LEN + en az CODE + CHECKSUM)
            break;
        }

        quint8 len = static_cast<quint8>(buffer.at(2));
        int totalSize = 2 + 1 + len + 1; // AA55 + LEN + (len bytes) + checksum

        if (buffer.size() < totalSize) {
            // Tam paket gelmemiş
            break;
        }

        QByteArray packet = buffer.left(totalSize);
        processPacket(packet);
        buffer.remove(0, totalSize);
    }
}

void Reader::processPacket(const QByteArray &packet) {
    if (packet.size() < 5) return;

    quint8 len  = static_cast<quint8>(packet.at(2));
    // Kontrol: paket boyutu len ile uyumlu mu?
    if (packet.size() < (2 + 1 + len + 1)) return;

    quint8 checksumByte = static_cast<quint8>(packet.at(packet.size() - 1));

    // Checksum hesaplama (LEN + payload)
    quint8 sum = 0;
    sum += len;
    for (int i = 3; i < 3 + len && i < packet.size() - 1; ++i) {
        sum += static_cast<quint8>(packet.at(i));
    }
    sum &= 0xFF;

    if (sum != checksumByte) {
        qWarning() << "Packet checksum mismatch. Beklenen:" << checksumByte << "Hesaplanan:" << sum;
        return;
    }

    quint8 code = static_cast<quint8>(packet.at(3));

    if (code == 21 && len >= 10) {
        // waveform değeri (örnek index'ler, cihaz protokolüne göre kontrol et)
        quint8 waveformVal = static_cast<quint8>(packet.at(5));
        if (waveformVal != 127) {
            static int lastValue = 0;
            int smooth = (lastValue + waveformVal) / 2;
            lastValue = smooth;

            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            m_waveformBuffer.enqueue(WaveformPoint(smooth, currentTime));

            // 20 saniyeden eski verileri temizle
            cleanOldData();

            // Ekran için güncelle (m_waveform doldur)
            updateDisplayWaveform();

            emit waveformChanged();
        }

        // SPO2 (örnek konum; cihaz protokolüne göre kontrol et)
        quint8 spo2  = static_cast<quint8>(packet.at(7));
        if (spo2 == 127) {
            m_spo2 = -1;
        } else {
            m_spo2 = static_cast<int>(spo2);
        }
        emit spo2Changed();

        // PR (örnek 2 byte)
        quint8 pr_msb = static_cast<quint8>(packet.at(8));
        quint8 pr_lsb = static_cast<quint8>(packet.at(9));
        int pr = (static_cast<int>(pr_msb) << 8) | static_cast<int>(pr_lsb);
        if (pr == 255) {
            m_pr = -1;
        } else {
            m_pr = pr;
        }
        emit prChanged();
    } else {
        // Diğer kodlar burada işlenebilir
    }
}

void Reader::cleanOldData() {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 twentySecondsAgo = currentTime - (20 * 1000); // 20 saniye

    while (!m_waveformBuffer.isEmpty() &&
           m_waveformBuffer.first().timestamp < twentySecondsAgo) {
        m_waveformBuffer.dequeue();
    }
}

void Reader::updateDisplayWaveform() {
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

// Freeze: portu kapat -> işletim sistemi buffer'ı uygulamaya gelmez
void Reader::freeze() {
    if (!m_frozen) {
        m_frozen = true;
        // Seri portu kapat (readyRead gelmez)
        closeSerialPort();
        // Geçici bufferları temizle
        buffer.clear();
        qDebug() << "🔒 WAVEFORM DONDURULDU (seri port kapatıldı)";
        emit frozenChanged();
    }
}

void Reader::unfreeze() {
    if (m_frozen) {
        // Tekrar açmayı dene
        bool ok = openSerialPort();
        if (!ok) {
            qWarning() << "unfreeze(): seri port açılamadı, freeze devam ediyor.";
            // Eğer açılamadıysa frozen durumunu koru ve bildir.
            // m_frozen true kalsın.
            emit frozenChanged();
            return;
        }

        m_frozen = false;
        // Freeze sırasında oluşmuş gereksiz local buffer'ı temizle
        buffer.clear();
        qDebug() << "🔓 WAVEFORM DEVAM EDİYOR (seri port yeniden açıldı)";
        emit frozenChanged();
    }
}

void Reader::toggleFreeze() {
    if (m_frozen) unfreeze();
    else freeze();
}

bool Reader::setResponseTime(int seconds) {
    qDebug() << "setResponseTime çağrıldı:" << seconds << "saniye";

    quint8 settingByte = 0x00; // Başlangıç değeri

    // Frequency ayarı (50Hz varsayılan): Bits 1,0 = 10
    settingByte |= 0x02; // 10 binary = 0x02

    // Mode ayarı (Adult varsayılan): Bits 4,3,2 = 100
    settingByte |= 0x10; // 100 binary shifted = 0x10

    // Response time ayarı: Bits 7,6,5
    switch (seconds) {
    case 4:
        settingByte |= 0x92;
        break;
    case 8:
        settingByte |= 0xB2;
        break;
    case 16:
        settingByte |= 0xD2;
        break;
    default:
        qWarning() << "Geçersiz response time:" << seconds << "- 4, 8 veya 16 olmalı";
        return false;
    }

    // Ayar paketini gönder
    sendSettingToBiolight(settingByte);
    qDebug() << "Response time ayarlandı:" << seconds << "saniye (Byte: 0x" << Qt::hex << settingByte << ")";
    return true;
}

void Reader::sendSettingToBiolight(quint8 data) {
    if (!serial.isOpen()) {
        qWarning() << "sendSettingToBiolight: seri port kapalı, paket gönderilemedi.";
        return;
    }

    // Protokol: AA55 LEN CODE DATA CHECKSUM
    QByteArray packet;
    packet.append(static_cast<char>(0xAA));
    packet.append(static_cast<char>(0x55));
    quint8 len = 0x02;
    packet.append(static_cast<char>(len));
    quint8 code = 0x06;
    packet.append(static_cast<char>(code));
    packet.append(static_cast<char>(data));
    quint8 checksum = (len + code + data) & 0xFF;
    packet.append(static_cast<char>(checksum));

    qint64 bytesWritten = serial.write(packet);
    serial.flush();

    qDebug() << "Biolight ayar paketi gönderildi:" << packet.toHex(' ')
             << "(" << bytesWritten << "bytes)";
}
