QT += 3drender-private opengl gamepad

# Generate PDBs for Release builds:
win32:QMAKE_CFLAGS_RELEASE += /Zi /GL
win32:QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

# Unlock all the fun toys on Windows:
win32:QMAKE_CXXFLAGS += /std:c++latest

# @todo Make this more generic, or remove it altogether.
unix:INCLUDEPATH += /usr/include/c++/7.2.0

INCLUDEPATH += $$PWD/Library

SOURCES += \
   $$PWD/Library/controller.cpp \
   $$PWD/Library/DataStructs/block.cpp \
   $$PWD/Library/DataStructs/fileInfo.cpp \
   $$PWD/Library/DataStructs/light.cpp \
   $$PWD/Library/DataStructs/precisePoint.cpp \
   $$PWD/Library/DataStructs/driveScanningParameters.cpp \
   $$PWD/Library/DataStructs/vizBlock.cpp \
   $$PWD/Library/DriveScanner/driveScanner.cpp \
   $$PWD/Library/DriveScanner/driveScanningUtilities.cpp \
   $$PWD/Library/DriveScanner/scanningWorker.cpp \
   $$PWD/Library/DriveScanner/scopedHandle.cpp \
   $$PWD/Library/HID/gamepad.cpp \
   $$PWD/Library/HID/keyboardManager.cpp \
   $$PWD/Library/Scene/baseAsset.cpp \
   $$PWD/Library/Scene/crosshairAsset.cpp \
   $$PWD/Library/Scene/debuggingRayAsset.cpp \
   $$PWD/Library/Scene/frustumAsset.cpp \
   $$PWD/Library/Scene/gridAsset.cpp \
   $$PWD/Library/Scene/lightMarkerAsset.cpp \
   $$PWD/Library/Scene/lineAsset.cpp \
   $$PWD/Library/Scene/originMarkerAsset.cpp \
   $$PWD/Library/Scene/treemapAsset.cpp \
   $$PWD/Library/Settings/settings.cpp \
   $$PWD/Library/Settings/settingsManager.cpp \
   $$PWD/Library/Viewport/camera.cpp \
   $$PWD/Library/Viewport/gamepadContextMenu.cpp \
   $$PWD/Library/Viewport/glCanvas.cpp \
   $$PWD/Library/Viewport/mouseContextMenu.cpp \
   $$PWD/Library/Visualizations/linuxFileMonitor.cpp \
   $$PWD/Library/Visualizations/squarifiedTreemap.cpp \
   $$PWD/Library/Visualizations/visualization.cpp \
   $$PWD/Library/Visualizations/windowsFileMonitor.cpp \
   $$PWD/Library/Windows/aboutDialog.cpp \
   $$PWD/Library/Windows/breakdownDialog.cpp \
   $$PWD/Library/Windows/mainWindow.cpp \
   $$PWD/Library/Windows/scanBreakdownModel.cpp

HEADERS += \
   $$PWD/Library/constants.h \
   $$PWD/Library/controller.h \
   $$PWD/Library/DataStructs/block.h \
   $$PWD/Library/DataStructs/fileInfo.h \
   $$PWD/Library/DataStructs/light.h \
   $$PWD/Library/DataStructs/driveScanningParameters.h \
   $$PWD/Library/DataStructs/precisePoint.h \
   $$PWD/Library/DataStructs/scanningProgress.hpp \
   $$PWD/Library/DataStructs/vizBlock.h \
   $$PWD/Library/DriveScanner/driveScanner.h \
   $$PWD/Library/DriveScanner/driveScanningUtilities.h \
   $$PWD/Library/DriveScanner/scanningWorker.h \
   $$PWD/Library/DriveScanner/scopedHandle.h \
   $$PWD/Library/DriveScanner/winHack.hpp \
   $$PWD/Library/HID/gamepad.h \
   $$PWD/Library/HID/keyboardManager.h \
   $$PWD/Library/literals.h \
   $$PWD/Library/Scene/baseAsset.h \
   $$PWD/Library/Scene/crosshairAsset.h \
   $$PWD/Library/Scene/debuggingRayAsset.h \
   $$PWD/Library/Scene/frustumAsset.h \
   $$PWD/Library/Scene/gridAsset.h \
   $$PWD/Library/Scene/lightMarkerAsset.h \
   $$PWD/Library/Scene/lineAsset.h \
   $$PWD/Library/Scene/originMarkerAsset.h \
   $$PWD/Library/Scene/treemapAsset.h \
   $$PWD/Library/Settings/preferencesMap.hpp \
   $$PWD/Library/Settings/settings.h \
   $$PWD/Library/Settings/settingsManager.h \
   $$PWD/Library/Utilities/colorGradient.hpp \
   $$PWD/Library/Utilities/ignoreUnused.hpp \
   $$PWD/Library/Utilities/operatingSystemSpecific.hpp \
   $$PWD/Library/Utilities/scopeExit.hpp \
   $$PWD/Library/Utilities/threadSafeQueue.hpp \
   $$PWD/Library/Utilities/utilities.hpp \
   $$PWD/Library/Utilities/viewFrustum.hpp \
   $$PWD/Library/Viewport/camera.h \
   $$PWD/Library/Viewport/gamepadContextMenu.h \
   $$PWD/Library/Viewport/glCanvas.h \
   $$PWD/Library/Viewport/mouseContextMenu.h \
   $$PWD/Library/Visualizations/linuxFileMonitor.h \
   $$PWD/Library/Visualizations/fileChangeNotification.hpp \
   $$PWD/Library/Visualizations/squarifiedTreemap.h \
   $$PWD/Library/Visualizations/visualization.h \
   $$PWD/Library/Visualizations/windowsFileMonitor.h \
   $$PWD/Library/Windows/aboutDialog.h \
   $$PWD/Library/Windows/breakdownDialog.h \
   $$PWD/Library/Windows/mainWindow.h \
   $$PWD/Library/Windows/scanBreakdownModel.h

FORMS += \
   $$PWD/Library/Windows/aboutDialog.ui \
   $$PWD/Library/Windows/breakdownDialog.ui \
   $$PWD/Library/Windows/mainWindow.ui

INCLUDEPATH += \
   $$PWD/../../boost_1_66_0/ \
   $$PWD/../Foreign/RapidJson/include \
   $$PWD/../Foreign/Spdlog/include \
   $$PWD/../Foreign/Stopwatch/source \
   $$PWD/../Foreign/Tree/source

DEPENDPATH += $$PWD/../../boost_1_66_0/stage/lib

DISTFILES += \
   $$PWD/Library/Shaders/visualizationFragmentShader.frag \
   $$PWD/Library/Shaders/visualizationVertexShader.vert \
   $$PWD/Library/Shaders/simpleLineFragmentShader.frag \
   $$PWD/Library/Shaders/simpleLineVertexShader.vert \
   $$PWD/Library/Shaders/shadowMapping.frag \
   $$PWD/Library/Shaders/shadowMapping.vert \
   $$PWD/Library/Shaders/texturePreview.vert \
   $$PWD/Library/Shaders/texturePreview.frag

RESOURCES += \
   $$PWD/Library/resources.qrc

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
