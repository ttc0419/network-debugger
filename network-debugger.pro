QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# disables all the APIs deprecated before Qt 6.0.1
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060001

SOURCES += \
    main.cpp \
    main-widget.cpp

HEADERS += \
    main-widget.h

FORMS += \
    main-widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
