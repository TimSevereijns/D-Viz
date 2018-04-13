#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T19:01:44
#
#-------------------------------------------------

QT += 3drender-private gui opengl gamepad

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets 3dcore

TARGET = D-Viz
TEMPLATE = app
CONFIG += c++1z

# Generate PDBs for Release builds:
win32:QMAKE_CFLAGS_RELEASE += /Zi /GL
win32:QMAKE_LFLAGS_RELEASE += /MAP /debug /opt:ref

# Unlock all the fun toys on Windows:
win32:QMAKE_CXXFLAGS += /std:c++latest

# @todo Make this more generic, or remove it altogether.
unix:INCLUDEPATH += /usr/include/c++/7.2.0

SOURCES += \
   controller.cpp \
   DataStructs/block.cpp \
   DataStructs/fileInfo.cpp \
   DataStructs/light.cpp \
   DataStructs/precisePoint.cpp \
   DataStructs/driveScanningParameters.cpp \
   DataStructs/vizBlock.cpp \
   DriveScanner/driveScanner.cpp \
   DriveScanner/scanningWorker.cpp \
   DriveScanner/scopedHandle.cpp \
   HID/gamepad.cpp \
   HID/keyboardManager.cpp \
   main.cpp \
   Scene/baseAsset.cpp \
   Scene/crosshairAsset.cpp \
   Scene/debuggingRayAsset.cpp \
   Scene/frustumAsset.cpp \
   Scene/gridAsset.cpp \
   Scene/lightMarkerAsset.cpp \
   Scene/lineAsset.cpp \
   Scene/originMarkerAsset.cpp \
   Scene/treemapAsset.cpp \
   Settings/settings.cpp \
   Settings/settingsManager.cpp \
   Viewport/camera.cpp \
   Viewport/gamepadContextMenu.cpp \
   Viewport/glCanvas.cpp \
   Viewport/mouseContextMenu.cpp \
   Visualizations/squarifiedTreemap.cpp \
   Visualizations/visualization.cpp \
   Visualizations/windowsFileMonitor.cpp \
   Windows/aboutDialog.cpp \
   Windows/breakdownDialog.cpp \
   Windows/mainWindow.cpp \
   Windows/scanBreakdownModel.cpp

HEADERS += \
   constants.h \
   controller.h \
   DataStructs/block.h \
   DataStructs/fileInfo.h \
   DataStructs/light.h \
   DataStructs/driveScanningParameters.h \
   DataStructs/precisePoint.h \
   DataStructs/scanningProgress.hpp \
   DataStructs/vizBlock.h \
   DriveScanner/driveScanner.h \
   DriveScanner/scanningWorker.h \
   DriveScanner/scopedHandle.h \
   DriveScanner/winHack.hpp \
   HID/gamepad.h \
   HID/keyboardManager.h \
   literals.h \
   Scene/baseAsset.h \
   Scene/crosshairAsset.h \
   Scene/debuggingRayAsset.h \
   Scene/frustumAsset.h \
   Scene/gridAsset.h \
   Scene/lightMarkerAsset.h \
   Scene/lineAsset.h \
   Scene/originMarkerAsset.h \
   Scene/treemapAsset.h \
   Settings/preferencesMap.hpp \
   Settings/settings.h \
   Settings/settingsManager.h \
   Utilities/colorGradient.hpp \
   Utilities/ignoreUnused.hpp \
   Utilities/operatingSystemSpecific.hpp \
   Utilities/scopeExit.hpp \
   Utilities/utilities.hpp \
   Utilities/viewFrustum.hpp \
   Viewport/camera.h \
   Viewport/gamepadContextMenu.h \
   Viewport/glCanvas.h \
   Viewport/mouseContextMenu.h \
   Visualizations/squarifiedTreemap.h \
   Visualizations/visualization.h \
   Visualizations/windowsFileMonitor.h \
   Windows/aboutDialog.h \
   Windows/breakdownDialog.h \
   Windows/mainWindow.h \
   Windows/scanBreakdownModel.h

FORMS += \
   Windows/aboutDialog.ui \
   Windows/breakdownDialog.ui \
   Windows/mainWindow.ui

INCLUDEPATH += \
   ../../boost_1_66_0/ \
   ../Foreign/RapidJson/include \
   ../Foreign/Spdlog/include \
   ../Foreign/Stopwatch/source \
   ../Foreign/Tree/source

DEPENDPATH += ../../boost_1_66_0/stage/lib

DISTFILES += \
   Shaders/visualizationFragmentShader.frag \
   Shaders/visualizationVertexShader.vert \
   Shaders/simpleLineFragmentShader.frag \
   Shaders/simpleLineVertexShader.vert \
   Shaders/shadowMapping.frag \
   Shaders/shadowMapping.vert \
   Shaders/texturePreview.vert \
   Shaders/texturePreview.frag

RESOURCES += \
   resources.qrc

DEFINES += \
   BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

win32:CONFIG(release, debug|release) {
   DEFINES += _WIN32_WINNT=0x0501
   LIBS += \
      -lXInput9_1_0 \
      -lShell32 \
      -lOle32 \
      -L$$PWD/../../boost_1_66_0/stage/lib \
      -llibboost_system-vc141-mt-x64-1_66
}

win32:CONFIG(debug, debug|release) {
   DEFINES += _WIN32_WINNT=0x0501
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

CONFIG(release, debug|release) {
   library_files.path += $$OUT_PWD/release
   library_files.files += $$(QTDIR)/bin/Qt53DCore.dll
   library_files.files += $$(QTDIR)/bin/Qt5Widgets.dll
   library_files.files += $$(QTDIR)/bin/Qt5Gui.dll
   library_files.files += $$(QTDIR)/bin/Qt5Core.dll

   plugin_files.path += $$OUT_PWD/release/platforms
   plugin_files.files += $$(QTDIR)/plugins/platforms/qwindows.dll
}
CONFIG(debug, debug|release) {
   library_files.path += $$OUT_PWD/debug
   library_files.files += $$(QTDIR)/bin/Qt53DCore.dll
   library_files.files += $$(QTDIR)/bin/Qt5Widgets.dll
   library_files.files += $$(QTDIR)/bin/Qt5Gui.dll
   library_files.files += $$(QTDIR)/bin/Qt5Core.dll

   plugin_files.path += $$OUT_PWD/release/platforms
   plugin_files.files += $$(QTDIR)/plugins/platforms/qwindows.dll
}

INSTALLS += library_files
INSTALLS += plugin_files
