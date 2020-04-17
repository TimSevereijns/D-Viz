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
   Mocks/mockView.h \
   cameraTests.h \
   controllerTests.h \
   fileSizeLiterals.hpp \
   filesystemObserverTests.h \
   Mocks\mockFileMonitor.h \
   modelTests.h \
   multiTestHarness.h \
   nodePainterTests.h \
   persistentSettingsTests.h \
   sessionSettingsTests.h \
   testUtilities.hpp \
   Mocks/mockFileMonitor.h \
   trompeloeilAdapter.h

SOURCES += \
   cameraTests.cpp \
   controllerTests.cpp \
   filesystemObserverTests.cpp \
   mainTest.cpp \
   modelTests.cpp \
   nodePainterTests.cpp \
   persistentSettingsTests.cpp \
   sessionSettingsTests.cpp

INCLUDEPATH += \
   $$PWD/../ThirdParty/Trompeloeil/include \

win32: CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}
