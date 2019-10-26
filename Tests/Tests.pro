#-------------------------------------------------
#
# Project created by QtCreator 2018-06-03T16:30:26
#
#-------------------------------------------------

include(../defaults.pri)

QT += testlib widgets
QT -= gui

unix: QMAKE_CXXFLAGS += --coverage
unix: QMAKE_LFLAGS += --coverage

TARGET = UnitTests

CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L$$DESTDIR -lD-Viz

HEADERS += \
   fileSizeLiterals.hpp \
   filesystemObserverTests.h \
   mockFileMonitor.h \
   modelTests.h \
   multiTestHarness.h \
   testUtilities.hpp

SOURCES += \
   filesystemObserverTests.cpp \
   mainTest.cpp \
   modelTests.cpp
