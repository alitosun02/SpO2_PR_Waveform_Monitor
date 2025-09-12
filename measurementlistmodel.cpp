#include "measurementlistmodel.h"
#include <QDebug>

MeasurementListModel::MeasurementListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_dbManager(new DatabaseManager(this))
    , m_currentPatientId(-1)
    , m_filterActive(false)
    , m_spo2Min(0), m_spo2Max(0), m_prMin(0), m_prMax(0)
    , m_addPatientPending(false)
{
    setupDatabaseConnections();

    // Başlangıçta verileri yükleme isteği gönder (database hazır olduğunda yüklenecek)
    if (m_dbManager->isReady()) {
        refreshData();
    }
}

void MeasurementListModel::setupDatabaseConnections()
{
    connect(m_dbManager, &DatabaseManager::databaseReady,
            this, &MeasurementListModel::onDatabaseReady);
    connect(m_dbManager, &DatabaseManager::patientAdded,
            this, &MeasurementListModel::onPatientAdded);
    connect(m_dbManager, &DatabaseManager::measurementSaved,
            this, &MeasurementListModel::onMeasurementSaved);
    connect(m_dbManager, &DatabaseManager::dataLoaded,
            this, &MeasurementListModel::onDataLoaded);
    connect(m_dbManager, &DatabaseManager::filteredDataLoaded,
            this, &MeasurementListModel::onFilteredDataLoaded);
    connect(m_dbManager, &DatabaseManager::error,
            this, &MeasurementListModel::onDatabaseError);
}

int MeasurementListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_data.count();
}

QVariant MeasurementListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count())
        return QVariant();

    const QVariantMap &item = m_data.at(index.row());

    switch (role) {
    case FirstNameRole: return item.value("first_name");
    case LastNameRole: return item.value("last_name");
    case Spo2Role: return item.value("spo2");
    case PrRole: return item.value("pr");
    case TimestampRole: return item.value("timestamp");
    default: return QVariant();
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
    if (m_addPatientPending) {
        qWarning() << "MeasurementListModel: Hasta ekleme işlemi zaten devam ediyor";
        return false;
    }

    if (firstName.trimmed().isEmpty() || lastName.trimmed().isEmpty()) {
        qWarning() << "MeasurementListModel: Ad ve soyad boş olamaz";
        return false;
    }

    m_addPatientPending = true;
    m_pendingFirstName = firstName;
    m_pendingLastName = lastName;

    // Async istek gönder
    m_dbManager->addPatient(firstName, lastName);
    return true; // İşlem başlatıldı anlamında true döner
}

bool MeasurementListModel::hasActivePatient() const
{
    return m_currentPatientId > 0;
}

void MeasurementListModel::saveMeasurement(int spo2, int pr)
{
    if (m_currentPatientId <= 0) {
        qWarning() << "MeasurementListModel: Aktif hasta yok, ölçüm kaydedilemiyor";
        return;
    }

    if (spo2 < 0 || spo2 > 100 || pr < 0 || pr > 300) {
        qWarning() << "MeasurementListModel: Geçersiz ölçüm değerleri - SpO2:" << spo2 << "PR:" << pr;
        return;
    }

    // Async istek gönder
    m_dbManager->saveMeasurement(m_currentPatientId, spo2, pr);
}

void MeasurementListModel::refreshData()
{
    if (!m_dbManager->isReady()) {
        qDebug() << "MeasurementListModel: Veritabanı henüz hazır değil, veri yükleme ertelendi";
        return;
    }

    if (m_filterActive) {
        m_dbManager->loadFilteredData(m_spo2Min, m_spo2Max, m_prMin, m_prMax);
    } else {
        m_dbManager->loadAllData();
    }
}

void MeasurementListModel::applyFilter(int spo2Min, int spo2Max, int prMin, int prMax)
{
    // Değerleri normalize et
    if (spo2Min > spo2Max) {
        int temp = spo2Min;
        spo2Min = spo2Max;
        spo2Max = temp;
    }

    if (prMin > prMax) {
        int temp = prMin;
        prMin = prMax;
        prMax = temp;
    }

    m_filterActive = true;
    m_spo2Min = spo2Min;
    m_spo2Max = spo2Max;
    m_prMin = prMin;
    m_prMax = prMax;

    qDebug() << "MeasurementListModel: Filtre uygulanıyor - SpO2:" << spo2Min << "-" << spo2Max
             << "PR:" << prMin << "-" << prMax;

    m_dbManager->loadFilteredData(spo2Min, spo2Max, prMin, prMax);
}

void MeasurementListModel::clearFilter()
{
    qDebug() << "MeasurementListModel: Filtre temizleniyor";

    m_filterActive = false;
    m_spo2Min = m_spo2Max = m_prMin = m_prMax = 0;
    m_dbManager->loadAllData();
}

QString MeasurementListModel::getLastPatientName() const
{
    if (m_data.isEmpty()) return "Hasta Bulunamadı";

    const QVariantMap &lastRecord = m_data.first();
    QString firstName = lastRecord.value("first_name").toString();
    QString lastName = lastRecord.value("last_name").toString();

    if (firstName.isEmpty() && lastName.isEmpty()) return "Hasta Bulunamadı";

    return (firstName + " " + lastName).trimmed();
}

// DatabaseManager slot handlers
void MeasurementListModel::onDatabaseReady()
{
    qDebug() << "MeasurementListModel: Veritabanı hazır, veriler yükleniyor";
    refreshData();
}

void MeasurementListModel::onPatientAdded(int newPatientId, bool success)
{
    m_addPatientPending = false;

    if (success && newPatientId > 0) {
        m_currentPatientId = newPatientId;
        qDebug() << "MeasurementListModel: Yeni hasta eklendi, ID:" << newPatientId
                 << "Ad:" << m_pendingFirstName << m_pendingLastName;
        emit activePatientChanged(true);
        refreshData(); // Veri listesini güncelle
    } else {
        qWarning() << "MeasurementListModel: Hasta eklenemedi -" << m_pendingFirstName << m_pendingLastName;
        emit activePatientChanged(false);
    }

    // Pending bilgileri temizle
    m_pendingFirstName.clear();
    m_pendingLastName.clear();
}

void MeasurementListModel::onMeasurementSaved(bool success)
{
    if (success) {
        qDebug() << "MeasurementListModel: Ölçüm başarıyla kaydedildi (Patient ID:" << m_currentPatientId << ")";
        refreshData(); // Veri listesini güncelle
    } else {
        qWarning() << "MeasurementListModel: Ölçüm kaydedilemedi (Patient ID:" << m_currentPatientId << ")";
    }
}

void MeasurementListModel::onDataLoaded(const QVariantList &data)
{
    qDebug() << "MeasurementListModel: Tüm veriler yüklendi, kayıt sayısı:" << data.size();
    updateModelData(data);
}

void MeasurementListModel::onFilteredDataLoaded(const QVariantList &data)
{
    qDebug() << "MeasurementListModel: Filtrelenmiş veriler yüklendi, kayıt sayısı:" << data.size();
    updateModelData(data);
}

void MeasurementListModel::onDatabaseError(const QString &message)
{
    qCritical() << "MeasurementListModel: Veritabanı hatası:" << message;

    // Hata durumunda pending işlemleri temizle
    if (m_addPatientPending) {
        m_addPatientPending = false;
        m_pendingFirstName.clear();
        m_pendingLastName.clear();
        emit activePatientChanged(false);
    }
}

void MeasurementListModel::updateModelData(const QVariantList &data)
{
    beginResetModel();
    m_data.clear();

    for (const QVariant &item : data) {
        if (item.canConvert<QVariantMap>()) {
            m_data.append(item.toMap());
        } else {
            qWarning() << "MeasurementListModel: Invalid QVariant in updateModelData";
        }
    }

    endResetModel();

    qDebug() << "MeasurementListModel: Model güncellendi, toplam kayıt:" << m_data.size();
}
