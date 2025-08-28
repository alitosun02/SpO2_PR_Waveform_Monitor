#include "measurementlistmodel.h"
#include <QDebug>

MeasurementListModel::MeasurementListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_dbManager(this)
    , m_currentPatientId(-1)
{
    // Başlangıçta mevcut verileri yükle
    refreshData();
}

int MeasurementListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.count();
}

QVariant MeasurementListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count())
        return QVariant();

    const QVariantMap &item = m_data.at(index.row());

    switch (role) {
    case FirstNameRole:
        return item.value("first_name");
    case LastNameRole:
        return item.value("last_name");
    case Spo2Role:
        return item.value("spo2");
    case PrRole:
        return item.value("pr");
    case TimestampRole:
        return item.value("timestamp");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MeasurementListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(FirstNameRole, "first_name");
    roles.insert(LastNameRole, "last_name");
    roles.insert(Spo2Role, "spo2");
    roles.insert(PrRole, "pr");
    roles.insert(TimestampRole, "timestamp");
    return roles;
}

bool MeasurementListModel::addPatient(const QString &firstName, const QString &lastName)
{
    qDebug() << "Model: addPatient çağrıldı -" << firstName << lastName;

    int newPatientId;
    if (m_dbManager.addPatient(firstName, lastName, newPatientId)) {
        m_currentPatientId = newPatientId;
        qDebug() << "Model: Aktif hasta ID set edildi:" << m_currentPatientId;
        qDebug() << "Model: hasActivePatient() şimdi:" << hasActivePatient();
        emit activePatientChanged(true);
        refreshData();
        return true;
    }
    return false;
}

bool MeasurementListModel::hasActivePatient() const
{
    return m_currentPatientId > 0;
}

void MeasurementListModel::saveMeasurement(int spo2, int pr)
{
    if (m_currentPatientId <= 0) {
        qWarning() << "Model: Ölçüm kaydedilemiyor - Aktif hasta yok!";
        return;
    }

    qDebug() << "Model: Ölçüm kaydediliyor - Hasta ID:" << m_currentPatientId << "SpO2:" << spo2 << "PR:" << pr;
    m_dbManager.saveMeasurement(m_currentPatientId, spo2, pr);
    refreshData();
}

void MeasurementListModel::refreshData()
{
    qDebug() << "Model: Veriler yenileniyor...";
    loadDataFromDatabase();
}

void MeasurementListModel::loadDataFromDatabase()
{
    beginResetModel();

    m_data.clear();
    QVariantList dbData = m_dbManager.getAllData();

    for (const QVariant &item : dbData) {
        m_data.append(item.toMap());
    }

    endResetModel();
    qDebug() << "Model: Yenilendi. Kayıt sayısı:" << m_data.count();
}

QString MeasurementListModel::getLastPatientName() const
{
    if (m_data.isEmpty()) {
        return "Hasta Bulunamadı";
    }

    const QVariantMap &lastRecord = m_data.first();
    QString firstName = lastRecord.value("first_name").toString();
    QString lastName = lastRecord.value("last_name").toString();

    if (firstName.isEmpty() && lastName.isEmpty()) {
        return "Hasta Bulunamadı";
    }

    return (firstName + " " + lastName).trimmed();
}
