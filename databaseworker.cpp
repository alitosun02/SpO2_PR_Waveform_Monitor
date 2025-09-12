#include "databaseworker.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QUuid>

DatabaseWorker::DatabaseWorker(QObject *parent)
    : QObject(parent)
{
    // Her worker için benzersiz bağlantı adı oluştur
    m_connectionName = QString("DatabaseWorker_%1").arg(QUuid::createUuid().toString());
}

DatabaseWorker::~DatabaseWorker()
{
    closeDatabase();
}

void DatabaseWorker::initializeDatabase()
{
    qDebug() << "DatabaseWorker::initializeDatabase - Thread ID:" << QThread::currentThreadId(); // <-- ekleme

    QMutexLocker locker(&m_mutex);

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName("patients.db");

    if (!m_db.open()) {
        QString errorMsg = QString("Veritabanı açılamadı: %1").arg(m_db.lastError().text());
        qCritical() << errorMsg;
        emit error(errorMsg);
        return;
    }

    if (!createTables()) {
        QString errorMsg = "Veritabanı tabloları oluşturulamadı";
        qCritical() << errorMsg;
        emit error(errorMsg);
        return;
    }

    qDebug() << "DatabaseWorker: Veritabanı başarıyla başlatıldı";
    emit databaseReady();
}

bool DatabaseWorker::createTables()
{
    QSqlQuery q(m_db);

    // Patients tablosu oluştur
    if (!q.exec("CREATE TABLE IF NOT EXISTS patients ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "first_name TEXT NOT NULL,"
                "last_name TEXT NOT NULL,"
                "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)")) {
        qCritical() << "Patients tablosu oluşturulamadı:" << q.lastError().text();
        return false;
    }

    // Measurements tablosu oluştur
    if (!q.exec("CREATE TABLE IF NOT EXISTS measurements ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "patient_id INTEGER NOT NULL,"
                "spo2 INTEGER NOT NULL,"
                "pr INTEGER NOT NULL,"
                "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
                "FOREIGN KEY(patient_id) REFERENCES patients(id))")) {
        qCritical() << "Measurements tablosu oluşturulamadı:" << q.lastError().text();
        return false;
    }

    return true;
}

void DatabaseWorker::addPatient(const QString &firstName, const QString &lastName)
{
    QMutexLocker locker(&m_mutex);

    if (firstName.isEmpty() || lastName.isEmpty()) {
        qWarning() << "DatabaseWorker: Ad ve soyad boş olamaz";
        emit patientAdded(-1, false);
        return;
    }

    if (!m_db.isOpen()) {
        qWarning() << "DatabaseWorker: Veritabanı bağlantısı kapalı";
        emit patientAdded(-1, false);
        return;
    }

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO patients (first_name, last_name) VALUES (:firstName, :lastName)");
    q.bindValue(":firstName", firstName);
    q.bindValue(":lastName", lastName);

    if (!q.exec()) {
        QString errorMsg = QString("Hasta eklenemedi: %1").arg(q.lastError().text());
        qCritical() << errorMsg;
        emit error(errorMsg);
        emit patientAdded(-1, false);
        return;
    }

    int newPatientId = q.lastInsertId().toInt();
    qDebug() << "DatabaseWorker: Yeni hasta eklendi, ID:" << newPatientId;
    emit patientAdded(newPatientId, true);
}

void DatabaseWorker::saveMeasurement(int patientId, int spo2, int pr)
{
    qDebug() << "DatabaseWorker::initializeDatabase - Thread ID:" << QThread::currentThreadId(); // <-- ekleme

    QMutexLocker locker(&m_mutex);

    if (patientId <= 0) {
        qWarning() << "DatabaseWorker: Geçersiz hasta ID:" << patientId;
        emit measurementSaved(false);
        return;
    }

    if (!m_db.isOpen()) {
        qWarning() << "DatabaseWorker: Veritabanı bağlantısı kapalı";
        emit measurementSaved(false);
        return;
    }

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO measurements (patient_id, spo2, pr) VALUES (:patientId, :spo2, :pr)");
    q.bindValue(":patientId", patientId);
    q.bindValue(":spo2", spo2);
    q.bindValue(":pr", pr);

    if (!q.exec()) {
        QString errorMsg = QString("Ölçüm kaydedilemedi: %1").arg(q.lastError().text());
        qCritical() << errorMsg;
        emit error(errorMsg);
        emit measurementSaved(false);
        return;
    }

    qDebug() << "DatabaseWorker: Ölçüm kaydedildi - PatientID:" << patientId << "SpO2:" << spo2 << "PR:" << pr;
    emit measurementSaved(true);
}

void DatabaseWorker::loadAllData()
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        qWarning() << "DatabaseWorker: Veritabanı bağlantısı kapalı";
        emit dataLoaded(QVariantList());
        return;
    }

    QVariantList list;
    QSqlQuery q(m_db);

    q.prepare("SELECT p.first_name, p.last_name, m.spo2, m.pr, "
              "strftime('%Y-%m-%d %H:%M:%S', m.timestamp) AS formatted_time "
              "FROM measurements m "
              "JOIN patients p ON m.patient_id = p.id "
              "ORDER BY m.timestamp DESC");

    if (!q.exec()) {
        QString errorMsg = QString("loadAllData SQL hatası: %1").arg(q.lastError().text());
        qCritical() << errorMsg;
        emit error(errorMsg);
        emit dataLoaded(QVariantList());
        return;
    }

    while (q.next()) {
        QVariantMap record;
        record["first_name"] = q.value("first_name");
        record["last_name"] = q.value("last_name");
        record["spo2"] = q.value("spo2");
        record["pr"] = q.value("pr");
        record["timestamp"] = q.value("formatted_time");
        list.append(record);
    }

    qDebug() << "DatabaseWorker: Tüm veriler yüklendi, kayıt sayısı:" << list.size();
    emit dataLoaded(list);
}

void DatabaseWorker::loadFilteredData(int spo2Min, int spo2Max, int prMin, int prMax)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        qWarning() << "DatabaseWorker: Veritabanı bağlantısı kapalı";
        emit filteredDataLoaded(QVariantList());
        return;
    }

    QVariantList list;
    QSqlQuery q(m_db);

    QString queryString = "SELECT p.first_name, p.last_name, m.spo2, m.pr, "
                          "strftime('%Y-%m-%d %H:%M:%S', m.timestamp) AS formatted_time "
                          "FROM measurements m "
                          "JOIN patients p ON m.patient_id = p.id "
                          "WHERE 1=1 ";

    if (spo2Min > 0) queryString += "AND m.spo2 >= :spo2Min ";
    if (spo2Max > 0 && spo2Max <= 100) queryString += "AND m.spo2 <= :spo2Max ";
    if (prMin > 0) queryString += "AND m.pr >= :prMin ";
    if (prMax > 0 && prMax <= 300) queryString += "AND m.pr <= :prMax ";

    queryString += "ORDER BY m.timestamp DESC";

    q.prepare(queryString);

    // Parametreleri güvenli şekilde bağla
    if (spo2Min > 0) q.bindValue(":spo2Min", spo2Min);
    if (spo2Max > 0 && spo2Max <= 100) q.bindValue(":spo2Max", spo2Max);
    if (prMin > 0) q.bindValue(":prMin", prMin);
    if (prMax > 0 && prMax <= 300) q.bindValue(":prMax", prMax);

    if (!q.exec()) {
        QString errorMsg = QString("loadFilteredData SQL hatası: %1").arg(q.lastError().text());
        qWarning() << errorMsg;
        emit error(errorMsg);
        emit filteredDataLoaded(QVariantList());
        return;
    }

    while (q.next()) {
        QVariantMap record;
        record["first_name"] = q.value("first_name");
        record["last_name"] = q.value("last_name");
        record["spo2"] = q.value("spo2");
        record["pr"] = q.value("pr");
        record["timestamp"] = q.value("formatted_time");
        list.append(record);
    }

    qDebug() << "DatabaseWorker: Filtrelenmiş veriler yüklendi, kayıt sayısı:" << list.size();
    emit filteredDataLoaded(list);
}

void DatabaseWorker::closeDatabase()
{
    QMutexLocker locker(&m_mutex);

    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "DatabaseWorker: Veritabanı bağlantısı kapatıldı";
    }

    // Bağlantıyı tamamen kaldır
    QSqlDatabase::removeDatabase(m_connectionName);
}
