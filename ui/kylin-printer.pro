QT += \
    core \
    gui  \
    dbus \
    printsupport \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = kylin-printer

VERSION = 1.0.1

DEFINES += QT_MESSAGELOGCONTEXT

target.path = /usr/bin
target.source += $$TARGET

# systemd.path = /usr/lib/systemd/user/
# systemd.files += $$PWD/data/kylin-printer.service

# startdesktop.files += kylin-printer.desktop
# startdesktop.path = /etc/xdg/autostart/

INSTALLS += \
    target  \
    # systemd \
    # startdesktop

INCLUDEPATH += \
    $$PWD/src/

SOURCES += \
    # $$PWD/src/main.cpp          \
    # $$PWD/src/deviceMonitor.cpp \
    $$PWD/src/*.cpp \
    manualinstallwindow.cpp \
    popwindow.cpp

HEADERS += \
    # $$PWD/src/deviceMonitor.h \
    $$PWD/src/*.h \
    manualinstallwindow.h \
    popwindow.h

RESOURCES += \
    picture.qrc