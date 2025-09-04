#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDebug>
#include "reader.h"
#include "databasemanager.h"
#include "measurementlistmodel.h"
#include "pdfexporter.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Model oluştur
    MeasurementListModel model;
    engine.rootContext()->setContextProperty("measurementModel", &model);

    // Reader heap üzerinde oluşturuluyor; app parent olarak veriliyor ki yaşam süresi boyunca canlı kalsın
    Reader *r = new Reader("COM8", &app);
    engine.rootContext()->setContextProperty("reader", r);

    // PDF Exporter oluştur
    PdfExporter pdfExporter;
    engine.rootContext()->setContextProperty("pdfExporter", &pdfExporter);

    // Timer ile 10 saniyede bir kayıt
    QTimer *saveTimer = new QTimer(&app);
    saveTimer->setInterval(10000); // 10 saniye
    saveTimer->setSingleShot(false);

    // Eğer aktif hasta varsa timer başlat, aksi halde durdur
    QObject::connect(&model, &MeasurementListModel::activePatientChanged, [&](bool ready) {
        if (ready) {
            if (!saveTimer->isActive()) {
                saveTimer->start();
                qDebug() << "🔔 Save timer started";
            }
        } else {
            if (saveTimer->isActive()) {
                saveTimer->stop();
                qDebug() << "🔕 Save timer stopped (no active patient)";
            }
        }
    });

    // Timer tetiklenince ölçüm kaydet
    QObject::connect(saveTimer, &QTimer::timeout, [&model, r]() {
        if (!model.hasActivePatient())
            return;

        // Freeze edildiğinde ölçüm kaydı da durdur
        bool frozen = r->property("frozen").toBool();
        if (frozen) {
            qDebug() << "⏸️ FREEZE - Ölçüm kaydı atlandı";
            return;
        }

        const int s = r->spo2();
        const int p = r->pr();
        if (s != -1 && p != -1) {
            model.saveMeasurement(s, p);
            qDebug() << "Saved measurement - SpO2:" << s << " PR:" << p;
        } else {
            qDebug() << "Measurement invalid (s or p == -1)";
        }
    });

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
