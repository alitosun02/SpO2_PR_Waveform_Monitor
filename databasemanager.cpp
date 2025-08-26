#include "databasemanager.h"

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

    // Patients tablosu
    if (!q.exec("CREATE TABLE IF NOT EXISTS patients ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "first_name TEXT NOT NULL,"
                "last_name TEXT NOT NULL,"
                "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)")) {
        qCritical() << "Patients tablosu oluşturulamadı:" << q.lastError().text();
        return false;
    }

    // Measurements tablosu
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
    qDebug() << "addPatient çağrıldı. Ad:" << firstName << "Soyad:" << lastName;

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
    qDebug() << "Hasta eklendi. ID =" << newPatientId;
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
    } else {
        qDebug() << "Ölçüm kaydedildi - Hasta ID:" << patientId << "SpO2:" << spo2 << "PR:" << pr;
    }
}

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
        record["first_name"] = q.value(0).toString();
        record["last_name"] = q.value(1).toString();
        record["spo2"] = q.value(2).toInt();
        record["pr"] = q.value(3).toInt();
        record["timestamp"] = q.value(4).toString();
        list.append(record);
    }

    qDebug() << "getAllData() - dönen kayıt sayısı:" << list.count();
    return list;
}
