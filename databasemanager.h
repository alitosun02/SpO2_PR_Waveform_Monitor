#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariantList>
#include <QVariantMap>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool addPatient(const QString &firstName, const QString &lastName, int &newPatientId);
    void saveMeasurement(int patientId, int spo2, int pr);
    QVariantList getAllData() const;

    // Yeni filtre fonksiyonları
    QVariantList getFilteredData(int spo2Min, int spo2Max, int prMin, int prMax) const;

    // Veritabanı nesnesine erişim
    QSqlDatabase& db() { return m_db; }
    const QSqlDatabase& db() const { return m_db; }

private:
    QSqlDatabase m_db;
    bool initializeDatabase();
};

#endif // DATABASEMANAGER_H
