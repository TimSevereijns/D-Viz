CONFIG += conan_basic_setup
include(../Conan/conanbuildinfo.pri)

QT += opengl gamepad

# Generate PDBs for Release builds:
win32:QMAKE_CFLAGS_RELEASE += /Zi /GL
win32:QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

INCLUDEPATH += $$PWD/Include

SOURCES += \
   $$PWD/Core/controller.cpp \
   $$PWD/Core/HID/gamepad.cpp \
   $$PWD/Core/HID/keyboardManager.cpp \
   $$PWD/Core/Scanner/driveScanner.cpp \
   $$PWD/Core/Scanner/fileInfo.cpp \
   $$PWD/Core/Scanner/scanningParameters.cpp \
   $$PWD/Core/Scanner/scanningUtilities.cpp \
   $$PWD/Core/Scanner/scanningWorker.cpp \
   $$PWD/Core/Scanner/Monitor/fileSystemObserver.cpp \
   $$PWD/Core/Scanner/Monitor/linuxFileMonitor.cpp \
   $$PWD/Core/Scanner/Monitor/windowsFileMonitor.cpp \
   $$PWD/Core/Scene/light.cpp \
   $$PWD/Core/Scene/Assets/baseAsset.cpp \
   $$PWD/Core/Scene/Assets/crosshairAsset.cpp \
   $$PWD/Core/Scene/Assets/debuggingRayAsset.cpp \
   $$PWD/Core/Scene/Assets/frustumAsset.cpp \
   $$PWD/Core/Scene/Assets/gridAsset.cpp \
   $$PWD/Core/Scene/Assets/lightMarkerAsset.cpp \
   $$PWD/Core/Scene/Assets/lineAsset.cpp \
   $$PWD/Core/Scene/Assets/originMarkerAsset.cpp \
   $$PWD/Core/Scene/Assets/treemapAsset.cpp \
   $$PWD/Core/Settings/settings.cpp \
   $$PWD/Core/Settings/settingsManager.cpp \
   $$PWD/Core/Utilities/scopedHandle.cpp \
   $$PWD/Core/Viewport/camera.cpp \
   $$PWD/Core/Viewport/gamepadContextMenu.cpp \
   $$PWD/Core/Viewport/glCanvas.cpp \
   $$PWD/Core/Viewport/mouseContextMenu.cpp \
   $$PWD/Core/Visualizations/block.cpp \
   $$PWD/Core/Visualizations/precisePoint.cpp \
   $$PWD/Core/Visualizations/ray.cpp \
   $$PWD/Core/Visualizations/squarifiedTreemap.cpp \
   $$PWD/Core/Visualizations/visualization.cpp \
   $$PWD/Core/Visualizations/vizBlock.cpp \
   $$PWD/Core/Windows/aboutDialog.cpp \
   $$PWD/Core/Windows/breakdownDialog.cpp \
   $$PWD/Core/Windows/mainWindow.cpp \
   $$PWD/Core/Windows/scanBreakdownModel.cpp

HEADERS += \
   $$PWD/Include/bootstrapper.hpp \
   $$PWD/Include/constants.h \
   $$PWD/Include/controller.h \
   $$PWD/Include/HID/gamepad.h \
   $$PWD/Include/HID/keyboardManager.h \
   $$PWD/Include/literals.h \
   $$PWD/Include/Scanner/driveScanner.h \
   $$PWD/Include/Scanner/fileInfo.h \
   $$PWD/Include/Scanner/scanningParameters.h \
   $$PWD/Include/Scanner/scanningProgress.hpp \
   $$PWD/Include/Scanner/scanningUtilities.h \
   $$PWD/Include/Scanner/scanningWorker.h \
   $$PWD/Include/Scanner/Monitor/fileMonitorBase.h \
   $$PWD/Include/Scanner/Monitor/fileSystemObserver.h \
   $$PWD/Include/Scanner/Monitor/linuxFileMonitor.h \
   $$PWD/Include/Scanner/Monitor/windowsFileMonitor.h \
   $$PWD/Include/Scene/light.h \
   $$PWD/Include/Scene/Assets/baseAsset.h \
   $$PWD/Include/Scene/Assets/crosshairAsset.h \
   $$PWD/Include/Scene/Assets/debuggingRayAsset.h \
   $$PWD/Include/Scene/Assets/frustumAsset.h \
   $$PWD/Include/Scene/Assets/gridAsset.h \
   $$PWD/Include/Scene/Assets/lightMarkerAsset.h \
   $$PWD/Include/Scene/Assets/lineAsset.h \
   $$PWD/Include/Scene/Assets/originMarkerAsset.h \
   $$PWD/Include/Scene/Assets/treemapAsset.h \
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
   $$PWD/Include/Utilities/scopedHandle.h \
   $$PWD/Include/Utilities/reparsePointDeclarations.hpp \
   $$PWD/Include/Viewport/camera.h \
   $$PWD/Include/Viewport/gamepadContextMenu.h \
   $$PWD/Include/Viewport/glCanvas.h \
   $$PWD/Include/Viewport/mouseContextMenu.h \
   $$PWD/Include/Visualizations/block.h \
   $$PWD/Include/Visualizations/precisePoint.h \
   $$PWD/Include/Visualizations/ray.h \
   $$PWD/Include/Visualizations/squarifiedTreemap.h \
   $$PWD/Include/Visualizations/visualization.h \
   $$PWD/Include/Visualizations/vizBlock.h \
   $$PWD/Include/Windows/aboutDialog.h \
   $$PWD/Include/Windows/breakdownDialog.h \
   $$PWD/Include/Windows/mainWindow.h \
   $$PWD/Include/Windows/scanBreakdownModel.h

FORMS += \
   $$PWD/Core/Windows/aboutDialog.ui \
   $$PWD/Core/Windows/breakdownDialog.ui \
   $$PWD/Core/Windows/mainWindow.ui

INCLUDEPATH += \
   $$PWD/../Foreign/RapidJson/include \
   $$PWD/../Foreign/Spdlog/include \
   $$PWD/../Foreign/Stopwatch/source \
   $$PWD/../Foreign/Tree/source \
   $$PWD/../Foreign/GSL/include

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
      _WIN32_WINNT=0x0601 \
      NOMINMAX
   LIBS += \
      -lXInput9_1_0 \
      -lShell32 \
      -lOle32
}

win32:CONFIG(debug, debug|release) {
   DEFINES += \
      _WIN32_WINNT=0x0601 \
      NOMINMAX
   LIBS += \
      -lXInput9_1_0 \
      -lShell32 \
      -lOle32
}

unix:CONFIG(release, debug|release) {
   LIBS += \
      -lstdc++fs
}

unix:CONFIG(debug, debug|release) {
   LIBS += \
      -lstdc++fs
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
