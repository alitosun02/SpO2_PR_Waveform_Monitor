#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);

    // QML'den çağrılabilmesi için Q_INVOKABLE
    Q_INVOKABLE void addPatient(const QString &firstName, const QString &lastName);

signals:
    void patientAdded(bool success);

private:
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
