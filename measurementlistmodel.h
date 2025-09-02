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
    Q_PROPERTY(bool hasActivePatient READ hasActivePatient NOTIFY activePatientChanged)

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
    Q_INVOKABLE QString getLastPatientName() const;
    Q_INVOKABLE bool addPatient(const QString &firstName, const QString &lastName);
    Q_INVOKABLE bool hasActivePatient() const;
    Q_INVOKABLE void refreshData();

    // Yeni filtre metodları
    Q_INVOKABLE void applyFilter(int spo2Min, int spo2Max, int prMin, int prMax);
    Q_INVOKABLE void clearFilter();

    // Ölçüm kaydetme
    void saveMeasurement(int spo2, int pr);

signals:
    void activePatientChanged(bool ready);

private:
    void loadDataFromDatabase();
    void loadFilteredDataFromDatabase(int spo2Min, int spo2Max, int prMin, int prMax);

    QList<QVariantMap> m_data;
    DatabaseManager m_dbManager;
    int m_currentPatientId;

    // Filtre durumu
    bool m_filterActive;
    int m_spo2Min, m_spo2Max, m_prMin, m_prMax;
};

#endif // MEASUREMENTLISTMODEL_H
