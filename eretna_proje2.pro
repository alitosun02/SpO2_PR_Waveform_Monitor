QT += core serialport sql quick qml quickcontrols2 charts

CONFIG += console c++17 qml_debug

SOURCES += main.cpp \
    databasemanager.cpp \
    measurementlistmodel.cpp \
    reader.cpp

HEADERS += \
    databasemanager.h \
    measurementlistmodel.h \
    reader.h

DISTFILES += \
    main.qml

RESOURCES += \
    qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
