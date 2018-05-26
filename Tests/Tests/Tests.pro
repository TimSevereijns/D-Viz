#-------------------------------------------------
#
# Project created by QtCreator 2018-05-21T21:57:37
#
#-------------------------------------------------

QT       += testlib 3drender-private gui opengl gamepad

QT       -= gui

TARGET = UnitTests
CONFIG   += console
CONFIG   -= app_bundle

# Generate PDBs for Release builds:
win32:QMAKE_CFLAGS_RELEASE += /Zi /GL
win32:QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

# Unlock all the fun toys on Windows:
win32:QMAKE_CXXFLAGS += /std:c++latest

# @todo Make this more generic, or remove it altogether.
unix:INCLUDEPATH += /usr/include/c++/7.2.0

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

SOURCES += \
    visualizationModelTests.cpp \
   ../../Source/Visualizations/visualization.cpp \
   ../../Source/Visualizations/squarifiedTreemap.cpp \
   ../../Source/DataStructs/block.cpp \
   ../../Source/DataStructs/fileInfo.cpp \
   ../../Source/DataStructs/light.cpp \
   ../../Source/DataStructs/precisePoint.cpp \
   ../../Source/DataStructs/driveScanningParameters.cpp \
   ../../Source/DataStructs/vizBlock.cpp \
   ../../Source/DriveScanner/driveScanner.cpp \
   ../../Source/DriveScanner/driveScanningUtilities.cpp \
   ../../Source/DriveScanner/scanningWorker.cpp \
   ../../Source/DriveScanner/scopedHandle.cpp \
   ../../Source/HID/gamepad.cpp \
   ../../Source/HID/keyboardManager.cpp \
   ../../Source/Scene/baseAsset.cpp \
   ../../Source/Scene/crosshairAsset.cpp \
   ../../Source/Scene/debuggingRayAsset.cpp \
   ../../Source/Scene/frustumAsset.cpp \
   ../../Source/Scene/gridAsset.cpp \
   ../../Source/Scene/lightMarkerAsset.cpp \
   ../../Source/Scene/lineAsset.cpp \
   ../../Source/Scene/originMarkerAsset.cpp \
   ../../Source/Scene/treemapAsset.cpp \
   ../../Source/Settings/settings.cpp \
   ../../Source/Settings/settingsManager.cpp \
   ../../Source/Viewport/camera.cpp \
   ../../Source/Viewport/gamepadContextMenu.cpp \
   ../../Source/Viewport/glCanvas.cpp \
   ../../Source/Viewport/mouseContextMenu.cpp \
   ../../Source/Visualizations/linuxFileMonitor.cpp \
   ../../Source/Visualizations/squarifiedTreemap.cpp \
   ../../Source/Visualizations/visualization.cpp \
   ../../Source/Visualizations/windowsFileMonitor.cpp \
   ../../Source/Windows/scanBreakdownModel.cpp

HEADERS += \
   ../../Source/Visualizations/visualization.h \
   ../../Source/Visualizations/squarifiedTreemap.h \
   ../../Source/DataStructs/block.h \
   ../../Source/DataStructs/fileInfo.h \
   ../../Source/DataStructs/light.h \
   ../../Source/DataStructs/driveScanningParameters.h \
   ../../Source/DataStructs/precisePoint.h \
   ../../Source/DataStructs/scanningProgress.hpp \
   ../../Source/DataStructs/vizBlock.h \
   ../../Source/DriveScanner/driveScanner.h \
   ../../Source/DriveScanner/driveScanningUtilities.h \
   ../../Source/DriveScanner/scanningWorker.h \
   ../../Source/DriveScanner/scopedHandle.h \
   ../../Source/DriveScanner/winHack.hpp \
   ../../Source/HID/gamepad.h \
   ../../Source/HID/keyboardManager.h \
   ../../Source/literals.h \
   ../../Source/Scene/baseAsset.h \
   ../../Source/Scene/crosshairAsset.h \
   ../../Source/Scene/debuggingRayAsset.h \
   ../../Source/Scene/frustumAsset.h \
   ../../Source/Scene/gridAsset.h \
   ../../Source/Scene/lightMarkerAsset.h \
   ../../Source/Scene/lineAsset.h \
   ../../Source/Scene/originMarkerAsset.h \
   ../../Source/Scene/treemapAsset.h \
   ../../Source/Settings/preferencesMap.hpp \
   ../../Source/Settings/settings.h \
   ../../Source/Settings/settingsManager.h \
   ../../Source/Utilities/colorGradient.hpp \
   ../../Source/Utilities/ignoreUnused.hpp \
   ../../Source/Utilities/operatingSystemSpecific.hpp \
   ../../Source/Utilities/scopeExit.hpp \
   ../../Source/Utilities/threadSafeQueue.hpp \
   ../../Source/Utilities/utilities.hpp \
   ../../Source/Utilities/viewFrustum.hpp \
   ../../Source/Viewport/camera.h \
   ../../Source/Viewport/gamepadContextMenu.h \
   ../../Source/Viewport/glCanvas.h \
   ../../Source/Viewport/mouseContextMenu.h \
   ../../Source/Visualizations/linuxFileMonitor.h \
   ../../Source/Visualizations/fileChangeNotification.hpp \
   ../../Source/Visualizations/squarifiedTreemap.h \
   ../../Source/Visualizations/visualization.h \
   ../../Source/Visualizations/windowsFileMonitor.h \
   ../../Source/Windows/scanBreakdownModel.h

INCLUDEPATH += \
   ../../Source/ \
   ../../../boost_1_66_0/ \
   ../../Foreign/RapidJson/include \
   ../../Foreign/Spdlog/include \
   ../../Foreign/Stopwatch/source \
   ../../Foreign/Tree/source

DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32:CONFIG(debug, debug|release) {
   DEFINES += \
      NOMINMAX
}
