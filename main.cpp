#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "reader.h"
#include "databasemanager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    DatabaseManager dbManager;
    engine.rootContext()->setContextProperty("dbManager", &dbManager);

    Reader r("COM8");
    engine.rootContext()->setContextProperty("reader", &r);

    // --- SADECE 10 SANİYEDE 1 KAYIT İÇİN TIMER ---
    auto saveTimer = new QTimer(&app);
    saveTimer->setInterval(10000);   // 10 saniye
    saveTimer->setSingleShot(false);

    // Hasta aktif olduğunda timer'ı başlat
    QObject::connect(&dbManager, &DatabaseManager::activePatientChanged, &app,
                     [&, saveTimer](bool ready){
                         if (!ready) return;
                         if (!saveTimer->isActive())
                             saveTimer->start();
                     });

    // Yalnızca timer tetiklenince en güncel SpO2 ve PR'ı kaydet
    QObject::connect(saveTimer, &QTimer::timeout, &app, [&](){
        if (!dbManager.hasActivePatient())
            return;

        const int s = r.spo2();
        const int p = r.pr();
        if (s != -1 && p != -1) {
            dbManager.saveMeasurement(s, p);
        }
    });
    // --- BİTTİ ---

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
