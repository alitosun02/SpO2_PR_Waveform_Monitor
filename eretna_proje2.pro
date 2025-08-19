QT += core serialport
QT += quick qml
CONFIG += console c++11
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

