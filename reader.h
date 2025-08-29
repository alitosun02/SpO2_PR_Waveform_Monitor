#ifndef READER_H
#define READER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QVariantList>
#include <QDateTime>
#include <QQueue>

struct WaveformPoint {
    double value;
    qint64 timestamp; // milliseconds since epoch

    WaveformPoint() : value(0), timestamp(0) {}
    WaveformPoint(double v, qint64 t) : value(v), timestamp(t) {}
};

class Reader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int spo2 READ spo2 NOTIFY spo2Changed)
    Q_PROPERTY(int pr READ pr NOTIFY prChanged)
    Q_PROPERTY(QVariantList waveform READ waveform NOTIFY waveformChanged)

public:
    explicit Reader(const QString &portName, QObject *parent = nullptr);

    int spo2() const { return m_spo2; }
    int pr() const { return m_pr; }
    QVariantList waveform() const { return m_waveform; }

    Q_INVOKABLE QVariantList getLast20SecondsWaveform() const;
    Q_INVOKABLE QVariantList getLast20SecondsTimestamps() const;

signals:
    void spo2Changed();
    void prChanged();
    void waveformChanged();

private slots:
    void readSerialData();
    void processPacket(const QByteArray &packet);

private:
    void cleanOldData(); // 20 saniyeden eski verileri temizle
    void updateDisplayWaveform(); // Ekran için waveform güncelle

private:
    QSerialPort serial;
    QByteArray buffer;

    int m_spo2 = -1;
    int m_pr = -1;
    QVariantList m_waveform; // Ekran için (son 200 nokta)

    // Yeni buffer sistemi - timestamp'li
    QQueue<WaveformPoint> m_waveformBuffer; // Tüm waveform geçmişi
    static const int MAX_DISPLAY_POINTS = 200; // Ekranda gösterilecek nokta sayısı
};

#endif // READER_H
