#ifndef MEASUREMENTLISTMODEL_H
#define MEASUREMENTLISTMODEL_H

#include <QAbstractListModel>
#include <QVariantMap>
#include <QSqlQuery>
#include <QSqlError>
#include "databasemanager.h"

class MeasurementListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit MeasurementListModel(QObject *parent = nullptr);

    enum Roles {
        FirstNameRole = Qt::UserRole + 1,
        LastNameRole,
        Spo2Role,
        PrRole,
        TimestampRole
    };

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // QML'den çağrılabilir metodlar
    Q_INVOKABLE bool addPatient(const QString &firstName, const QString &lastName);
    Q_INVOKABLE bool hasActivePatient() const;
    Q_INVOKABLE void refreshData();

    // Ölçüm kaydetme
    void saveMeasurement(int spo2, int pr);

signals:
    void activePatientChanged(bool ready);

private:
    void loadDataFromDatabase();

    QList<QVariantMap> m_data;
    DatabaseManager m_dbManager;
    int m_currentPatientId;
};

#endif // MEASUREMENTLISTMODEL_H
