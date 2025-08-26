#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "reader.h"
#include "databasemanager.h"
#include "measurementlistmodel.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Model oluştur
    MeasurementListModel model;
    engine.rootContext()->setContextProperty("measurementModel", &model);

    // Reader oluştur
    Reader r("COM8");
    engine.rootContext()->setContextProperty("reader", &r);

    // Timer ile 10 saniyede bir kayıt
    auto saveTimer = new QTimer(&app);
    saveTimer->setInterval(10000); // 10 saniye
    saveTimer->setSingleShot(false);

    // Hasta eklendiğinde timer'ı başlat
    QObject::connect(&model, &MeasurementListModel::activePatientChanged, [&](bool ready) {
        if (ready && !saveTimer->isActive()) {
            saveTimer->start();
        }
    });

    // Timer tetiklenince ölçüm kaydet
    QObject::connect(saveTimer, &QTimer::timeout, [&]() {
        if (!model.hasActivePatient())
            return;

        const int s = r.spo2();
        const int p = r.pr();
        if (s != -1 && p != -1) {
            model.saveMeasurement(s, p);
        }
    });

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
