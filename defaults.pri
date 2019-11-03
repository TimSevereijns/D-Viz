CONFIG += \
    conan_basic_setup
    force_debug_info

include(Conan/conanbuildinfo.pri)

QT += opengl gamepad

INCLUDEPATH += $$PWD/Include

SOURCES += \
   $$PWD/Source/controller.cpp \
   $$PWD/Source/HID/gamepad.cpp \
   $$PWD/Source/HID/keyboardManager.cpp \
   $$PWD/Source/Scanner/driveScanner.cpp \
   $$PWD/Source/Scanner/fileInfo.cpp \
   $$PWD/Source/Scanner/scanningParameters.cpp \
   $$PWD/Source/Scanner/scanningUtilities.cpp \
   $$PWD/Source/Scanner/scanningWorker.cpp \
   $$PWD/Source/Scanner/Monitor/fileSystemObserver.cpp \
   $$PWD/Source/Scanner/Monitor/linuxFileMonitor.cpp \
   $$PWD/Source/Scanner/Monitor/windowsFileMonitor.cpp \
   $$PWD/Source/Scene/light.cpp \
   $$PWD/Source/Scene/Assets/baseAsset.cpp \
   $$PWD/Source/Scene/Assets/crosshairAsset.cpp \
   $$PWD/Source/Scene/Assets/debuggingRayAsset.cpp \
   $$PWD/Source/Scene/Assets/frustumAsset.cpp \
   $$PWD/Source/Scene/Assets/gridAsset.cpp \
   $$PWD/Source/Scene/Assets/lightMarkerAsset.cpp \
   $$PWD/Source/Scene/Assets/lineAsset.cpp \
   $$PWD/Source/Scene/Assets/originMarkerAsset.cpp \
   $$PWD/Source/Scene/Assets/treemapAsset.cpp \
   $$PWD/Source/Settings/settings.cpp \
   $$PWD/Source/Settings/settingsManager.cpp \
   $$PWD/Source/Utilities/scopedHandle.cpp \
   $$PWD/Source/Viewport/camera.cpp \
   $$PWD/Source/Viewport/gamepadContextMenu.cpp \
   $$PWD/Source/Viewport/glCanvas.cpp \
   $$PWD/Source/Viewport/mouseContextMenu.cpp \
   $$PWD/Source/Visualizations/block.cpp \
   $$PWD/Source/Visualizations/precisePoint.cpp \
   $$PWD/Source/Visualizations/ray.cpp \
   $$PWD/Source/Visualizations/squarifiedTreemap.cpp \
   $$PWD/Source/Visualizations/visualization.cpp \
   $$PWD/Source/Visualizations/vizBlock.cpp \
   $$PWD/Source/Windows/aboutDialog.cpp \
   $$PWD/Source/Windows/breakdownDialog.cpp \
   $$PWD/Source/Windows/mainWindow.cpp \
   $$PWD/Source/Windows/scanBreakdownModel.cpp

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
   $$PWD/Source/Windows/aboutDialog.ui \
   $$PWD/Source/Windows/breakdownDialog.ui \
   $$PWD/Source/Windows/mainWindow.ui

INCLUDEPATH += \
   $$PWD/ThirdParty/Rapidjson/include \
   $$PWD/ThirdParty/Spdlog/include \
   $$PWD/ThirdParty/Stopwatch/source \
   $$PWD/ThirdParty/Tree/source \
   $$PWD/ThirdParty/GSL/include

DISTFILES += \
   $$PWD/Source/Shaders/visualizationFragmentShader.frag \
   $$PWD/Source/Shaders/visualizationVertexShader.vert \
   $$PWD/Source/Shaders/simpleLineFragmentShader.frag \
   $$PWD/Source/Shaders/simpleLineVertexShader.vert \
   $$PWD/Source/Shaders/shadowMapping.frag \
   $$PWD/Source/Shaders/shadowMapping.vert \
   $$PWD/Source/Shaders/texturePreview.vert \
   $$PWD/Source/Shaders/texturePreview.frag

RESOURCES += \
   $$PWD/Source/resources.qrc

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
   QMAKE_CXXFLAGS += --coverage
   QMAKE_LFLAGS += --coverage
   LIBS += \
      -lstdc++fs
}

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/Output/Debug
} else {
    DESTDIR = $$PWD/Output/Release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
