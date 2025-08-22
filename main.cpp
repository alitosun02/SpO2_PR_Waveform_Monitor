#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "reader.h"
#include "databasemanager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    DatabaseManager dbManager;
    engine.rootContext()->setContextProperty("dbManager", &dbManager);

    Reader r("COM8");
    engine.rootContext()->setContextProperty("reader", &r);

    // Hasta eklendiğinde ölçüm kaydını devreye al
    QObject::connect(&dbManager, &DatabaseManager::activePatientChanged, &app,
                     [&](bool ready){
                         if (!ready) return;

                         // Sadece bir kez bağlayalım: spo2Changed tetiklendiğinde kaydet
                         QObject::connect(&r, &Reader::spo2Changed, &app, [&](){
                             if (dbManager.hasActivePatient() && r.spo2() != -1 && r.pr() != -1) {
                                 dbManager.saveMeasurement(r.spo2(), r.pr());
                             }
                         });
                     });

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
