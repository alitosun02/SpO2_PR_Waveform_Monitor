#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {
    // SQLite sürücüsü ile varsayılan bağlantı oluşturulur
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("patients.db");
    if (!db.open()) {
        qWarning() << "Veritabanı açılamadı:" << db.lastError().text();
    }

    // Tablo oluşturma (eğer yoksa)
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS patients ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "firstname TEXT, lastname TEXT)");
}

void DatabaseManager::addPatient(const QString &firstName, const QString &lastName) {
    QSqlQuery query;
    query.prepare("INSERT INTO patients (firstname, lastname) VALUES (:first, :last)");
    query.bindValue(":first", firstName);
    query.bindValue(":last", lastName);
    if (!query.exec()) {
        qWarning() << "Hasta eklenemedi:" << query.lastError().text();
        emit patientAdded(false);
    } else {
        emit patientAdded(true);
    }
}
