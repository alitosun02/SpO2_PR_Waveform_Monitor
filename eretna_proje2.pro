QT += core serialport
QT += quick qml
QT += quick charts serialport
QT += quick quickcontrols2 serialport
CONFIG += console c++11
CONFIG += qml_debug
SOURCES += main.cpp \
    reader.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    reader.h

DISTFILES += \
    main.qml

RESOURCES += \
    qml.qrc

