#ifndef READER_H
#define READER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QVariantList>

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

signals:
    void spo2Changed();
    void prChanged();
    void waveformChanged();

private slots:
    void readSerialData();
    void processPacket(const QByteArray &packet);

private:
    QSerialPort serial;
    QByteArray buffer;

    int m_spo2 = -1;
    int m_pr = -1;
    QVariantList m_waveform;
};

#endif // READER_H
