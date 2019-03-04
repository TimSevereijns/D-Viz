QT += opengl gamepad

# Generate PDBs for Release builds:
win32:QMAKE_CFLAGS_RELEASE += /Zi /GL
win32:QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

# Unlock all the fun toys on Windows:
win32:QMAKE_CXXFLAGS += /std:c++17

INCLUDEPATH += $$PWD/Include

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
   $$PWD/Core/Visualizations/fileSystemObserver.cpp \
   $$PWD/Core/Visualizations/ray.cpp \
   $$PWD/Core/Visualizations/squarifiedTreemap.cpp \
   $$PWD/Core/Visualizations/visualization.cpp \
   $$PWD/Core/Visualizations/windowsFileMonitor.cpp \
   $$PWD/Core/Windows/aboutDialog.cpp \
   $$PWD/Core/Windows/breakdownDialog.cpp \
   $$PWD/Core/Windows/mainWindow.cpp \
   $$PWD/Core/Windows/scanBreakdownModel.cpp

HEADERS += \
   $$PWD/Include/bootstrapper.hpp \
   $$PWD/Include/constants.h \
   $$PWD/Include/controller.h \
   $$PWD/Include/DataStructs/block.h \
   $$PWD/Include/DataStructs/fileInfo.h \
   $$PWD/Include/DataStructs/light.h \
   $$PWD/Include/DataStructs/driveScanningParameters.h \
   $$PWD/Include/DataStructs/precisePoint.h \
   $$PWD/Include/DataStructs/scanningProgress.hpp \
   $$PWD/Include/DataStructs/vizBlock.h \
   $$PWD/Include/DriveScanner/driveScanner.h \
   $$PWD/Include/DriveScanner/driveScanningUtilities.h \
   $$PWD/Include/DriveScanner/scanningWorker.h \
   $$PWD/Include/DriveScanner/scopedHandle.h \
   $$PWD/Include/DriveScanner/winHack.hpp \
   $$PWD/Include/HID/gamepad.h \
   $$PWD/Include/HID/keyboardManager.h \
   $$PWD/Include/literals.h \
   $$PWD/Include/Scene/baseAsset.h \
   $$PWD/Include/Scene/crosshairAsset.h \
   $$PWD/Include/Scene/debuggingRayAsset.h \
   $$PWD/Include/Scene/frustumAsset.h \
   $$PWD/Include/Scene/gridAsset.h \
   $$PWD/Include/Scene/lightMarkerAsset.h \
   $$PWD/Include/Scene/lineAsset.h \
   $$PWD/Include/Scene/originMarkerAsset.h \
   $$PWD/Include/Scene/treemapAsset.h \
   $$PWD/Include/Settings/preferencesMap.hpp \
   $$PWD/Include/Settings/settings.h \
   $$PWD/Include/Settings/settingsManager.h \
   $$PWD/Include/Utilities/colorGradient.hpp \
   $$PWD/Include/Utilities/ignoreUnused.hpp \
   $$PWD/Include/Utilities/operatingSystemSpecific.hpp \
   $$PWD/Include/Utilities/scopeExit.hpp \
   $$PWD/Include/Utilities/threadSafeQueue.hpp \
   $$PWD/Include/Utilities/utilities.hpp \
   $$PWD/Include/Utilities/viewFrustum.hpp \
   $$PWD/Include/Viewport/camera.h \
   $$PWD/Include/Viewport/gamepadContextMenu.h \
   $$PWD/Include/Viewport/glCanvas.h \
   $$PWD/Include/Viewport/mouseContextMenu.h \
   $$PWD/Include/Visualizations/linuxFileMonitor.h \
   $$PWD/Include/Visualizations/fileChangeNotification.hpp \
   $$PWD/Include/Visualizations/fileSystemObserver.h \
   $$PWD/Include/Visualizations/fileMonitorImpl.h \
   $$PWD/Include/Visualizations/ray.h \
   $$PWD/Include/Visualizations/squarifiedTreemap.h \
   $$PWD/Include/Visualizations/visualization.h \
   $$PWD/Include/Visualizations/windowsFileMonitor.h \
   $$PWD/Include/Windows/aboutDialog.h \
   $$PWD/Include/Windows/breakdownDialog.h \
   $$PWD/Include/Windows/mainWindow.h \
   $$PWD/Include/Windows/scanBreakdownModel.h

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

DEPENDPATH += \
   $$PWD/../../boost_1_66_0/stage/lib

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
      -L$$PWD/../Foreign/INotify-Cpp/build/src \
      -lboost_system \
      -lboost_filesystem \
      -linotify-cpp
}

unix:CONFIG(debug, debug|release) {
   LIBS += \
      -lstdc++fs \
      -L$$PWD/../../boost_1_66_0/stage/lib \
      -L$$PWD/../Foreign/INotify-Cpp/build/src \
      -lboost_system \
      -lboost_filesystem \
      -linotify-cpp
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
