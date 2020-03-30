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
   modelTests.h \
   multiTestHarness.h \
   sessionSettingsTests.h \
   testUtilities.hpp \
   Mocks\mockFileMonitor.h

SOURCES += \
   cameraTests.cpp \
   filesystemObserverTests.cpp \
   mainTest.cpp \
   modelTests.cpp \
   sessionSettingsTests.cpp \

win32: CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}
