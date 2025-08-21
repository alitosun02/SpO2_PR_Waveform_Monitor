#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "reader.h"
#include "databasemanager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Reader r("COM8"); // Bağlı cihazının COM portunu buradan değiştir
    engine.rootContext()->setContextProperty("reader", &r);

    DatabaseManager dbManager;
    engine.rootContext()->setContextProperty("dbManager", &dbManager);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
