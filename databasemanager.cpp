#include "databasemanager.h"
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_isReady(false)
{
    setupWorker();
}

DatabaseManager::~DatabaseManager()
{
    if (m_workerThread) {
        m_workerThread->quit();
        if (!m_workerThread->wait(3000)) {
            qWarning() << "DatabaseManager: Worker thread sonlandırılamadı, zorla kapatılıyor";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }

        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }

    if (m_worker) {
        m_worker->deleteLater();
        m_worker = nullptr;
    }
}

void DatabaseManager::setupWorker()
{
    // Worker thread oluştur
    m_workerThread = new QThread(this);
    m_worker = new DatabaseWorker();

    // Worker'ı thread'e taşı
    m_worker->moveToThread(m_workerThread);

    // Sinyalleri bağla - Manager'dan Worker'a
    connect(this, &DatabaseManager::initializeDatabase,
            m_worker, &DatabaseWorker::initializeDatabase);
    connect(this, &DatabaseManager::requestAddPatient,
            m_worker, &DatabaseWorker::addPatient);
    connect(this, &DatabaseManager::requestSaveMeasurement,
            m_worker, &DatabaseWorker::saveMeasurement);
    connect(this, &DatabaseManager::requestLoadAllData,
            m_worker, &DatabaseWorker::loadAllData);
    connect(this, &DatabaseManager::requestLoadFilteredData,
            m_worker, &DatabaseWorker::loadFilteredData);

    // Sinyalleri bağla - Worker'dan Manager'a (ve dışarı aktar)
    connect(m_worker, &DatabaseWorker::databaseReady, this, [this]() {
        m_isReady = true;
        emit databaseReady();
        qDebug() << "DatabaseManager: Veritabanı hazır";
    });

    connect(m_worker, &DatabaseWorker::patientAdded,
            this, &DatabaseManager::patientAdded);
    connect(m_worker, &DatabaseWorker::measurementSaved,
            this, &DatabaseManager::measurementSaved);
    connect(m_worker, &DatabaseWorker::dataLoaded,
            this, &DatabaseManager::dataLoaded);
    connect(m_worker, &DatabaseWorker::filteredDataLoaded,
            this, &DatabaseManager::filteredDataLoaded);
    connect(m_worker, &DatabaseWorker::error,
            this, &DatabaseManager::error);

    // Thread temizleme
    connect(m_workerThread, &QThread::finished,
            m_worker, &DatabaseWorker::deleteLater);

    // Thread'i başlat ve veritabanını initialize et
    m_workerThread->start();

    // Veritabanını başlat
    emit initializeDatabase();

    qDebug() << "DatabaseManager: Worker thread başlatıldı";
}

void DatabaseManager::addPatient(const QString &firstName, const QString &lastName)
{
    if (!m_isReady) {
        qWarning() << "DatabaseManager: Veritabanı henüz hazır değil";
        emit patientAdded(-1, false);
        return;
    }

    emit requestAddPatient(firstName, lastName);
}

void DatabaseManager::saveMeasurement(int patientId, int spo2, int pr)
{
    if (!m_isReady) {
        qWarning() << "DatabaseManager: Veritabanı henüz hazır değil";
        emit measurementSaved(false);
        return;
    }

    emit requestSaveMeasurement(patientId, spo2, pr);
}

void DatabaseManager::loadAllData()
{
    if (!m_isReady) {
        qWarning() << "DatabaseManager: Veritabanı henüz hazır değil";
        emit dataLoaded(QVariantList());
        return;
    }

    emit requestLoadAllData();
}

void DatabaseManager::loadFilteredData(int spo2Min, int spo2Max, int prMin, int prMax)
{
    if (!m_isReady) {
        qWarning() << "DatabaseManager: Veritabanı henüz hazır değil";
        emit filteredDataLoaded(QVariantList());
        return;
    }

    emit requestLoadFilteredData(spo2Min, spo2Max, prMin, prMax);
}
