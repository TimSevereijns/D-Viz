QT += gui opengl #gamepad

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets 3dcore

TARGET = D-Viz
TEMPLATE = subdirs

CONFIG += \
    c++17 \
    file_copies \
    ordered

SUBDIRS += \
   Source \
   Tests \
   App \
   Installer

App.depends = Source
Tests.depends = Source
Installer.depends = App
