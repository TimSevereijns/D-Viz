CONFIG += \
    conan_basic_setup
    force_debug_info

include(Conan/conanbuildinfo.pri)

QT += opengl gamepad charts
win32: QT += winextras

INCLUDEPATH += $$PWD/Include

SOURCES += \
    $$PWD/Source/controller.cpp \
    $$PWD/Source/Model/baseModel.cpp \
    $$PWD/Source/Model/block.cpp \
    $$PWD/Source/Model/Monitor/fileSystemObserver.cpp \
    $$PWD/Source/Model/Monitor/linuxFileMonitor.cpp \
    $$PWD/Source/Model/Monitor/windowsFileMonitor.cpp \
    $$PWD/Source/Model/precisePoint.cpp \
    $$PWD/Source/Model/ray.cpp \
    $$PWD/Source/Model/Scanner/driveScanner.cpp \
    $$PWD/Source/Model/Scanner/scanningParameters.cpp \
    $$PWD/Source/Model/Scanner/scanningUtilities.cpp \
    $$PWD/Source/Model/Scanner/scanningWorker.cpp \
    $$PWD/Source/Model/squarifiedTreemap.cpp \
    $$PWD/Source/Model/vizBlock.cpp \
    $$PWD/Source/Settings/nodePainter.cpp \
    $$PWD/Source/Settings/persistentSettings.cpp \
    $$PWD/Source/Settings/sessionSettings.cpp \
    $$PWD/Source/Settings/settings.cpp \
    $$PWD/Source/Utilities/scopedHandle.cpp \
    $$PWD/Source/View/Dialogs/aboutDialog.cpp \
    $$PWD/Source/View/Dialogs/breakdownDialog.cpp \
    $$PWD/Source/View/Dialogs/distributionGraphModel.cpp \
    $$PWD/Source/View/Dialogs/scanBreakdownModel.cpp \
    $$PWD/Source/View/HID/gamepad.cpp \
    $$PWD/Source/View/HID/keyboardManager.cpp \
    $$PWD/Source/View/mainWindow.cpp \
    $$PWD/Source/View/Scene/Assets/baseAsset.cpp \
    $$PWD/Source/View/Scene/Assets/crosshairAsset.cpp \
    $$PWD/Source/View/Scene/Assets/debuggingRayAsset.cpp \
    $$PWD/Source/View/Scene/Assets/frustumAsset.cpp \
    $$PWD/Source/View/Scene/Assets/gridAsset.cpp \
    $$PWD/Source/View/Scene/Assets/lightMarkerAsset.cpp \
    $$PWD/Source/View/Scene/Assets/lineAsset.cpp \
    $$PWD/Source/View/Scene/Assets/originMarkerAsset.cpp \
    $$PWD/Source/View/Scene/Assets/treemapAsset.cpp \
    $$PWD/Source/View/Scene/light.cpp \
    $$PWD/Source/View/Viewport/camera.cpp \
    $$PWD/Source/View/Viewport/gamepadContextMenu.cpp \
    $$PWD/Source/View/Viewport/glCanvas.cpp \
    $$PWD/Source/View/Viewport/mouseContextMenu.cpp

HEADERS += \
    $$PWD/Include/bootstrapper.hpp \
    $$PWD/Include/constants.h \
    $$PWD/Include/controller.h \
    $$PWD/Include/Factories/modelFactory.h \
    $$PWD/Include/Factories/modelFactoryInterface.h \
    $$PWD/Include/Factories/viewFactory.h \
    $$PWD/Include/Factories/viewFactoryInterface.h \
    $$PWD/Include/literals.h \
    $$PWD/Include/Model/baseModel.h \
    $$PWD/Include/Model/block.h \
    $$PWD/Include/Model/Monitor/fileMonitorBase.h \
    $$PWD/Include/Model/Monitor/fileSystemObserver.h \
    $$PWD/Include/Model/Monitor/linuxFileMonitor.h \
    $$PWD/Include/Model/Monitor/windowsFileMonitor.h \
    $$PWD/Include/Model/precisePoint.h \
    $$PWD/Include/Model/ray.h \
    $$PWD/Include/Model/Scanner/driveScanner.h \
    $$PWD/Include/Model/Scanner/fileInfo.h \
    $$PWD/Include/Model/Scanner/scanningParameters.h \
    $$PWD/Include/Model/Scanner/scanningProgress.hpp \
    $$PWD/Include/Model/Scanner/scanningUtilities.h \
    $$PWD/Include/Model/Scanner/scanningWorker.h \
    $$PWD/Include/Model/squarifiedTreemap.h \
    $$PWD/Include/Model/vizBlock.h \
    $$PWD/Include/Settings/nodePainter.h \
    $$PWD/Include/Settings/persistentSettings.h \
    $$PWD/Include/Settings/sessionSettings.h \
    $$PWD/Include/Settings/settings.h \
    $$PWD/Include/Utilities/ignoreUnused.hpp \
    $$PWD/Include/Utilities/operatingSystem.hpp \
    $$PWD/Include/Utilities/reparsePointDeclarations.hpp \
    $$PWD/Include/Utilities/scopedCursor.h \
    $$PWD/Include/Utilities/scopedHandle.h \
    $$PWD/Include/Utilities/scopeExit.hpp \
    $$PWD/Include/Utilities/threadSafeQueue.hpp \
    $$PWD/Include/Utilities/utilities.hpp \
    $$PWD/Include/Utilities/viewFrustum.hpp \
    $$PWD/Include/View/baseView.h \
    $$PWD/Include/View/Dialogs/aboutDialog.h \
    $$PWD/Include/View/Dialogs/breakdownDialog.h \
    $$PWD/Include/View/Dialogs/distributionGraphModel.h \
    $$PWD/Include/View/HID/gamepad.h \
    $$PWD/Include/View/HID/keyboardManager.h \
    $$PWD/Include/View/mainWindow.h \
    $$PWD/Include/View/scanBreakdownModel.h \
    $$PWD/Include/View/Scene/Assets/baseAsset.h \
    $$PWD/Include/View/Scene/Assets/crosshairAsset.h \
    $$PWD/Include/View/Scene/Assets/debuggingRayAsset.h \
    $$PWD/Include/View/Scene/Assets/frustumAsset.h \
    $$PWD/Include/View/Scene/Assets/gridAsset.h \
    $$PWD/Include/View/Scene/Assets/lightMarkerAsset.h \
    $$PWD/Include/View/Scene/Assets/lineAsset.h \
    $$PWD/Include/View/Scene/Assets/originMarkerAsset.h \
    $$PWD/Include/View/Scene/Assets/treemapAsset.h \
    $$PWD/Include/View/Scene/light.h \
    $$PWD/Include/View/Viewport/camera.h \
    $$PWD/Include/View/Viewport/gamepadContextMenu.h \
    $$PWD/Include/View/Viewport/glCanvas.h \
    $$PWD/Include/View/Viewport/mouseContextMenu.h

FORMS += \
    $$PWD/Source/View/Dialogs/aboutDialog.ui \
    $$PWD/Source/View/Dialogs/breakdownDialog.ui \
    $$PWD/Source/View/mainWindow.ui

INCLUDEPATH += \
    $$PWD/ThirdParty/Rapidjson/include \
    $$PWD/ThirdParty/Spdlog/include \
    $$PWD/ThirdParty/Stopwatch/source \
    $$PWD/ThirdParty/Tree/source \
    $$PWD/ThirdParty/GSL/include

DISTFILES += \
    $$PWD/Source/View/Shaders/visualizationFragmentShader.frag \
    $$PWD/Source/View/Shaders/visualizationVertexShader.vert \
    $$PWD/Source/View/Shaders/simpleLineFragmentShader.frag \
    $$PWD/Source/View/Shaders/simpleLineVertexShader.vert \
    $$PWD/Source/View/Shaders/shadowMapping.frag \
    $$PWD/Source/View/Shaders/shadowMapping.vert \
    $$PWD/Source/View/Shaders/texturePreview.vert \
    $$PWD/Source/View/Shaders/texturePreview.frag

RESOURCES += \
    $$PWD/Source/resources.qrc

DEFINES += \
    BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

win32: CONFIG(release, debug|release) {
    DEFINES += \
        _WIN32_WINNT=0x0601 \
        NOMINMAX
    LIBS += \
        -lXInput9_1_0 \
        -lShell32 \
        -lOle32
}

win32: CONFIG(debug, debug|release) {
    DEFINES += \
        _WIN32_WINNT=0x0601 \
        NOMINMAX
    LIBS += \
        -lXInput9_1_0 \
        -lShell32 \
        -lOle32
}

unix: CONFIG(release, debug|release) {
    LIBS += \
        -lstdc++fs
}

unix: CONFIG(debug, debug|release) {
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
