#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariantList>
#include <QVariantMap>
#include <QMutex>

class DatabaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseWorker(QObject *parent = nullptr);
    ~DatabaseWorker();

public slots:
    void initializeDatabase();
    void addPatient(const QString &firstName, const QString &lastName);
    void saveMeasurement(int patientId, int spo2, int pr);
    void loadAllData();
    void loadFilteredData(int spo2Min, int spo2Max, int prMin, int prMax);

signals:
    void databaseReady();
    void patientAdded(int newPatientId, bool success);
    void measurementSaved(bool success);
    void dataLoaded(const QVariantList &data);
    void filteredDataLoaded(const QVariantList &data);
    void error(const QString &message);

private:
    QSqlDatabase m_db;
    QMutex m_mutex;
    QString m_connectionName;

    bool createTables();
    void closeDatabase();
};

#endif // DATABASEWORKER_H
