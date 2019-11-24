include(../defaults.pri)

QT += testlib widgets
QT -= gui

TARGET = UnitTests

CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += \
    QT_DEPRECATED_WARNINGS
    SRCDIR=\\\"$$PWD/\\\"

LIBS += -L$$DESTDIR -lD-Viz

HEADERS += \
   cameraTests.h \
   fileSizeLiterals.hpp \
   filesystemObserverTests.h \
   mockFileMonitor.h \
   modelTests.h \
   multiTestHarness.h \
   testUtilities.hpp

SOURCES += \
   cameraTests.cpp \
   filesystemObserverTests.cpp \
   mainTest.cpp \
   modelTests.cpp
