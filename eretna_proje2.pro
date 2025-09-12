QT += core serialport sql quick qml quickcontrols2 charts printsupport


CONFIG += console c++17 qml_debug

SOURCES += main.cpp \
    databasemanager.cpp \
    databaseworker.cpp \
    measurementlistmodel.cpp \
    reader.cpp \
    pdfexporter.cpp

HEADERS += \
    databasemanager.h \
    databaseworker.h \
    measurementlistmodel.h \
    reader.h \
    pdfexporter.h

DISTFILES += \
    main.qml

RESOURCES += \
    qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
