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

RC_ICONS += D-Viz.ico
