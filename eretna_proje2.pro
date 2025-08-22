QT += core serialport
QT += quick qml
QT += quick charts serialport
QT += quick quickcontrols2 serialport
QT += sql
QT += core serialport sql
QT += quick qml quickcontrols2
CONFIG += console c++17
CONFIG += qml_debug
SOURCES += main.cpp \
    databasemanager.cpp \
    reader.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    databasemanager.h \
    reader.h

DISTFILES += \
    main.qml

RESOURCES += \
    qml.qrc

