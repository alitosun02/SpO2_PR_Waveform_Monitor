#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QThread>
#include <QVariantList>
#include <QVariantMap>
#include <QMutex>
#include "databaseworker.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Async metodlar - thread'e istek gönderir
    void addPatient(const QString &firstName, const QString &lastName);
    void saveMeasurement(int patientId, int spo2, int pr);
    void loadAllData();
    void loadFilteredData(int spo2Min, int spo2Max, int prMin, int prMax);

    // Durum kontrolü
    bool isReady() const { return m_isReady; }

signals:
    // DatabaseWorker'dan gelen sinyalleri dışarı aktar
    void databaseReady();
    void patientAdded(int newPatientId, bool success);
    void measurementSaved(bool success);
    void dataLoaded(const QVariantList &data);
    void filteredDataLoaded(const QVariantList &data);
    void error(const QString &message);

    // Worker'a sinyal gönder
    void initializeDatabase();
    void requestAddPatient(const QString &firstName, const QString &lastName);
    void requestSaveMeasurement(int patientId, int spo2, int pr);
    void requestLoadAllData();
    void requestLoadFilteredData(int spo2Min, int spo2Max, int prMin, int prMax);

private:
    QThread *m_workerThread;
    DatabaseWorker *m_worker;
    bool m_isReady;

    void setupWorker();
};

#endif // DATABASEMANAGER_H
