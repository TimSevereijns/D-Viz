include(../defaults.pri)

QT += core gui

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = D-Viz
TEMPLATE = app
CONFIG += c++1z

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
   main.cpp

LIBS += -L$$DESTDIR -lD-Viz

win32: CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}

win32: RC_ICONS += Icons/Windows/D-Viz.ico

unix {
    icon32.files = Icons/Linux/32x32/D-Viz.png
    icon32.path = /usr/share/icons/hicolor/32x32/apps
    icon64.files = Icons/Linux/64x64/D-Viz.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    icon128.files = Icons/Linux/128x128/D-Viz.png
    icon128.path = /usr/share/icons/hicolor/128x128/apps
    icon256.files = Icons/Linux/256x256/D-Viz.png
    icon256.path = /usr/share/icons/hicolor/256x256/apps

    INSTALLS += icon32 icon64 icon128 icon256
}
