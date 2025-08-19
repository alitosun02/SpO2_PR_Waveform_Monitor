#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "reader.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Reader nesnesini oluştur
    Reader r("COM8");

    // Reader’ı QML'e tanıt
    engine.rootContext()->setContextProperty("reader", &r);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
