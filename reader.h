#ifndef READER_H
#define READER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class Reader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int spo2 READ spo2 NOTIFY spo2Changed)
    Q_PROPERTY(int pr READ pr NOTIFY prChanged)

public:
    explicit Reader(const QString &portName, QObject *parent = nullptr);

    int spo2() const { return m_spo2; }
    int pr() const { return m_pr; }

signals:
    void spo2Changed();
    void prChanged();

private slots:
    void readSerialData();
    void processPacket(const QByteArray &packet);

private:
    QSerialPort serial;
    QByteArray buffer;

    int m_spo2 = -1;
    int m_pr = -1;
};

#endif // READER_H
