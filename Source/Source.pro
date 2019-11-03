include(../defaults.pri)

QT += core opengl

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = D-Viz
TEMPLATE = lib
CONFIG += staticlib c++17

DEFINES += QT_DEPRECATED_WARNINGS
