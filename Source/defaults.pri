QT += 3drender-private opengl gamepad

# Generate PDBs for Release builds:
win32:QMAKE_CFLAGS_RELEASE += /Zi /GL
win32:QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

# Unlock all the fun toys on Windows:
win32:QMAKE_CXXFLAGS += /std:c++latest

# @todo Make this more generic, or remove it altogether.
unix:INCLUDEPATH += /usr/include/c++/7.2.0

INCLUDEPATH += $$PWD/Core

SOURCES += \
   $$PWD/Core/controller.cpp \
   $$PWD/Core/DataStructs/block.cpp \
   $$PWD/Core/DataStructs/fileInfo.cpp \
   $$PWD/Core/DataStructs/light.cpp \
   $$PWD/Core/DataStructs/precisePoint.cpp \
   $$PWD/Core/DataStructs/driveScanningParameters.cpp \
   $$PWD/Core/DataStructs/vizBlock.cpp \
   $$PWD/Core/DriveScanner/driveScanner.cpp \
   $$PWD/Core/DriveScanner/driveScanningUtilities.cpp \
   $$PWD/Core/DriveScanner/scanningWorker.cpp \
   $$PWD/Core/DriveScanner/scopedHandle.cpp \
   $$PWD/Core/HID/gamepad.cpp \
   $$PWD/Core/HID/keyboardManager.cpp \
   $$PWD/Core/Scene/baseAsset.cpp \
   $$PWD/Core/Scene/crosshairAsset.cpp \
   $$PWD/Core/Scene/debuggingRayAsset.cpp \
   $$PWD/Core/Scene/frustumAsset.cpp \
   $$PWD/Core/Scene/gridAsset.cpp \
   $$PWD/Core/Scene/lightMarkerAsset.cpp \
   $$PWD/Core/Scene/lineAsset.cpp \
   $$PWD/Core/Scene/originMarkerAsset.cpp \
   $$PWD/Core/Scene/treemapAsset.cpp \
   $$PWD/Core/Settings/settings.cpp \
   $$PWD/Core/Settings/settingsManager.cpp \
   $$PWD/Core/Viewport/camera.cpp \
   $$PWD/Core/Viewport/gamepadContextMenu.cpp \
   $$PWD/Core/Viewport/glCanvas.cpp \
   $$PWD/Core/Viewport/mouseContextMenu.cpp \
   $$PWD/Core/Visualizations/linuxFileMonitor.cpp \
   $$PWD/Core/Visualizations/squarifiedTreemap.cpp \
   $$PWD/Core/Visualizations/visualization.cpp \
   $$PWD/Core/Visualizations/windowsFileMonitor.cpp \
   $$PWD/Core/Windows/aboutDialog.cpp \
   $$PWD/Core/Windows/breakdownDialog.cpp \
   $$PWD/Core/Windows/mainWindow.cpp \
   $$PWD/Core/Windows/scanBreakdownModel.cpp

HEADERS += \
   $$PWD/Core/bootstrapper.hpp \
   $$PWD/Core/constants.h \
   $$PWD/Core/controller.h \
   $$PWD/Core/DataStructs/block.h \
   $$PWD/Core/DataStructs/fileInfo.h \
   $$PWD/Core/DataStructs/light.h \
   $$PWD/Core/DataStructs/driveScanningParameters.h \
   $$PWD/Core/DataStructs/precisePoint.h \
   $$PWD/Core/DataStructs/scanningProgress.hpp \
   $$PWD/Core/DataStructs/vizBlock.h \
   $$PWD/Core/DriveScanner/driveScanner.h \
   $$PWD/Core/DriveScanner/driveScanningUtilities.h \
   $$PWD/Core/DriveScanner/scanningWorker.h \
   $$PWD/Core/DriveScanner/scopedHandle.h \
   $$PWD/Core/DriveScanner/winHack.hpp \
   $$PWD/Core/HID/gamepad.h \
   $$PWD/Core/HID/keyboardManager.h \
   $$PWD/Core/literals.h \
   $$PWD/Core/Scene/baseAsset.h \
   $$PWD/Core/Scene/crosshairAsset.h \
   $$PWD/Core/Scene/debuggingRayAsset.h \
   $$PWD/Core/Scene/frustumAsset.h \
   $$PWD/Core/Scene/gridAsset.h \
   $$PWD/Core/Scene/lightMarkerAsset.h \
   $$PWD/Core/Scene/lineAsset.h \
   $$PWD/Core/Scene/originMarkerAsset.h \
   $$PWD/Core/Scene/treemapAsset.h \
   $$PWD/Core/Settings/preferencesMap.hpp \
   $$PWD/Core/Settings/settings.h \
   $$PWD/Core/Settings/settingsManager.h \
   $$PWD/Core/Utilities/colorGradient.hpp \
   $$PWD/Core/Utilities/ignoreUnused.hpp \
   $$PWD/Core/Utilities/operatingSystemSpecific.hpp \
   $$PWD/Core/Utilities/scopeExit.hpp \
   $$PWD/Core/Utilities/threadSafeQueue.hpp \
   $$PWD/Core/Utilities/utilities.hpp \
   $$PWD/Core/Utilities/viewFrustum.hpp \
   $$PWD/Core/Viewport/camera.h \
   $$PWD/Core/Viewport/gamepadContextMenu.h \
   $$PWD/Core/Viewport/glCanvas.h \
   $$PWD/Core/Viewport/mouseContextMenu.h \
   $$PWD/Core/Visualizations/linuxFileMonitor.h \
   $$PWD/Core/Visualizations/fileChangeNotification.hpp \
   $$PWD/Core/Visualizations/fileMonitorBase.h \
   $$PWD/Core/Visualizations/squarifiedTreemap.h \
   $$PWD/Core/Visualizations/visualization.h \
   $$PWD/Core/Visualizations/windowsFileMonitor.h \
   $$PWD/Core/Windows/aboutDialog.h \
   $$PWD/Core/Windows/breakdownDialog.h \
   $$PWD/Core/Windows/mainWindow.h \
   $$PWD/Core/Windows/scanBreakdownModel.h

FORMS += \
   $$PWD/Core/Windows/aboutDialog.ui \
   $$PWD/Core/Windows/breakdownDialog.ui \
   $$PWD/Core/Windows/mainWindow.ui

INCLUDEPATH += \
   $$PWD/../../boost_1_66_0/ \
   $$PWD/../Foreign/RapidJson/include \
   $$PWD/../Foreign/Spdlog/include \
   $$PWD/../Foreign/Stopwatch/source \
   $$PWD/../Foreign/Tree/source \
   $$PWD/../Foreign/GSL/include

DEPENDPATH += $$PWD/../../boost_1_66_0/stage/lib

DISTFILES += \
   $$PWD/Core/Shaders/visualizationFragmentShader.frag \
   $$PWD/Core/Shaders/visualizationVertexShader.vert \
   $$PWD/Core/Shaders/simpleLineFragmentShader.frag \
   $$PWD/Core/Shaders/simpleLineVertexShader.vert \
   $$PWD/Core/Shaders/shadowMapping.frag \
   $$PWD/Core/Shaders/shadowMapping.vert \
   $$PWD/Core/Shaders/texturePreview.vert \
   $$PWD/Core/Shaders/texturePreview.frag

RESOURCES += \
   $$PWD/Core/resources.qrc

DEFINES += \
   BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

win32:CONFIG(release, debug|release) {
   DEFINES += \
      _WIN32_WINNT=0x0501 \
      NOMINMAX
   LIBS += \
      -lXInput9_1_0 \
      -lShell32 \
      -lOle32 \
      -L$$PWD/../../boost_1_66_0/stage/lib \
      -llibboost_system-vc141-mt-x64-1_66
}

win32:CONFIG(debug, debug|release) {
   DEFINES += \
      _WIN32_WINNT=0x0501 \
      NOMINMAX
   LIBS += \
      -lXInput9_1_0 \
      -lShell32 \
      -lOle32 \
      -L$$PWD/../../boost_1_66_0/stage/lib \
      -llibboost_system-vc141-mt-gd-x64-1_66
}

unix:CONFIG(release, debug|release) {
   LIBS += \
      -lstdc++fs \
      -L$$PWD/../../boost_1_66_0/stage/lib \
      -lboost_system
}

unix:CONFIG(debug, debug|release) {
   LIBS += \
      -lstdc++fs \
      -L$$PWD/../../boost_1_66_0/stage/lib \
      -lboost_system
}

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/Build/Debug
} else {
    DESTDIR = $$PWD/Build/Release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
