#include <QCoreApplication>
#include <QDebug>
#include <qTimer>
#include "reader.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Reader r("COM8"); // Port adı buradan değiştirilebilir
    QTimer::singleShot(1000, &a, &QCoreApplication::quit); // 1 saniye sonra kapan
    return a.exec();
}
