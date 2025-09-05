#include "databasemanager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("patients.db");

    if (!m_db.open()) {
        qCritical() << "Veritabanı açılamadı:" << m_db.lastError().text();
        return false;
    }

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

    qDebug() << "Veritabanı başarıyla başlatıldı";
    return true;
}

bool DatabaseManager::addPatient(const QString &firstName, const QString &lastName, int &newPatientId)
{
    if (firstName.isEmpty() || lastName.isEmpty()) {
        qWarning() << "Ad ve soyad boş olamaz";
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO patients (first_name, last_name) VALUES (:firstName, :lastName)");
    q.bindValue(":firstName", firstName);
    q.bindValue(":lastName", lastName);

    if (!q.exec()) {
        qCritical() << "Hasta eklenemedi:" << q.lastError().text();
        return false;
    }

    newPatientId = q.lastInsertId().toInt();
    return true;
}

void DatabaseManager::saveMeasurement(int patientId, int spo2, int pr)
{
    if (patientId <= 0) {
        qWarning() << "Geçersiz hasta ID:" << patientId;
        return;
    }

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO measurements (patient_id, spo2, pr) VALUES (:patientId, :spo2, :pr)");
    q.bindValue(":patientId", patientId);
    q.bindValue(":spo2", spo2);
    q.bindValue(":pr", pr);

    if (!q.exec()) {
        qCritical() << "Ölçüm kaydedilemedi:" << q.lastError().text();
    }
}

// Tüm verileri güvenli şekilde döndür
QVariantList DatabaseManager::getAllData() const
{
    QVariantList list;
    QSqlQuery q(m_db);

    q.prepare("SELECT p.first_name, p.last_name, m.spo2, m.pr, "
              "strftime('%Y-%m-%d %H:%M:%S', m.timestamp) AS formatted_time "
              "FROM measurements m "
              "JOIN patients p ON m.patient_id = p.id "
              "ORDER BY m.timestamp DESC");

    if (!q.exec()) {
        qCritical() << "getAllData() SQL hatası:" << q.lastError().text();
        return list;
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

    return list;
}

// FİLTRELENMİŞ VERİLERİ GÜVENLİ ŞEKİLDE DÖNDÜR
QVariantList DatabaseManager::getFilteredData(int spo2Min, int spo2Max, int prMin, int prMax) const
{
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
        qWarning() << "getFilteredData SQL Error:" << q.lastError().text();
        return list;
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

    return list;
}
