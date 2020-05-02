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
   controllerTests.h \
   fileSizeLiterals.hpp \
   filesystemObserverTests.h \
   modelTests.h \
   nodePainterTests.h \
   persistentSettingsTests.h \
   sessionSettingsTests.h \
   Mocks/mockView.h \
   Mocks/mockFileMonitor.h \
   Utilities/multiTestHarness.h \
   Utilities/testUtilities.hpp \
   Utilities/trompeloeilAdapter.h

SOURCES += \
   cameraTests.cpp \
   controllerTests.cpp \
   filesystemObserverTests.cpp \
   modelTests.cpp \
   nodePainterTests.cpp \
   persistentSettingsTests.cpp \
   sessionSettingsTests.cpp \
   testMain.cpp

INCLUDEPATH += \
   $$PWD/../ThirdParty/Trompeloeil/include \

win32: CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}
