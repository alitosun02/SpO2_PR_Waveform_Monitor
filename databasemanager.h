#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariantList>
#include <QVariantMap>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    Q_INVOKABLE bool addPatient(const QString &firstName, const QString &lastName);
    void saveMeasurement(int spo2, int pr);

    bool hasActivePatient() const { return m_currentPatientId != -1; }

    // --- YENİ EKLENEN ---
    Q_INVOKABLE QVariantList getRecentData() const;

signals:
    void activePatientChanged(bool ready);

private:
    QSqlDatabase m_db;
    int m_currentPatientId = -1; // aktif hasta ID
};

#endif // DATABASEMANAGER_H
