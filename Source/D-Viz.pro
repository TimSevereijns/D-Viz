#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T19:01:44
#
#-------------------------------------------------

QT += 3drender-private gui opengl gamepad

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets 3dcore

TARGET = D-Viz
TEMPLATE = subdirs
CONFIG += c++1z

INSTALLS += library_files
INSTALLS += plugin_files

CONFIG += ordered
SUBDIRS += \
   Core \
   Tests \
   App

App.depends = Core
Tests.depends = Core
