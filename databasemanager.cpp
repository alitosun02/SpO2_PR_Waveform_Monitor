#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("patients.db"); // proje klasöründe

    if (!m_db.open()) {
        qCritical() << "Veritabanı açılamadı:" << m_db.lastError().text();
        return;
    }

    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS patients ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "first_name TEXT,"
           "last_name TEXT,"
           "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)");

    q.exec("CREATE TABLE IF NOT EXISTS measurements ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "patient_id INTEGER,"
           "spo2 INTEGER,"
           "pr INTEGER,"
           "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
           "FOREIGN KEY(patient_id) REFERENCES patients(id))");
}

bool DatabaseManager::addPatient(const QString &firstName, const QString &lastName) {
    qDebug() << "addPatient çağrıldı. Ad:" << firstName << "Soyad:" << lastName;
    QSqlQuery q;
    q.prepare("INSERT INTO patients (first_name, last_name) VALUES (:f, :l)");
    q.bindValue(":f", firstName);
    q.bindValue(":l", lastName);

    if (!q.exec()) {
        qCritical() << "Hasta eklenemedi:" << q.lastError().text();
        return false;
    }

    m_currentPatientId = q.lastInsertId().toInt();
    qDebug() << "Hasta eklendi. ID =" << m_currentPatientId;

    emit activePatientChanged(true);
    return true;
}

void DatabaseManager::saveMeasurement(int spo2, int pr) {
    if (m_currentPatientId == -1) {
        // uyarı spam’ini engellemek için bir kere yazdırmak istersen:
        static bool warned = false;
        if (!warned) {
            qWarning() << "Önce hasta eklenmeli!";
            warned = true;
        }
        return;
    }

    QSqlQuery q;
    q.prepare("INSERT INTO measurements (patient_id, spo2, pr) "
              "VALUES (:pid, :s, :p)");
    q.bindValue(":pid", m_currentPatientId);
    q.bindValue(":s", spo2);
    q.bindValue(":p", pr);

    if (!q.exec()) {
        qCritical() << "Ölçüm kaydedilemedi:" << q.lastError().text();
    }
}
