#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T19:01:44
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets 3dcore

TARGET = D-Viz
TEMPLATE = app
CONFIG += c++11

SOURCES += \
   controller.cpp \
   DataStructs/block.cpp \
   DataStructs/fileInfo.cpp \
   DataStructs/light.cpp \
   DataStructs/precisePoint.cpp \
   DataStructs/vizNode.cpp \
   DataStructs/driveScanningParameters.cpp \
   DriveScanner/driveScanner.cpp \
   DriveScanner/scanningWorker.cpp \
   HID/keyboardManager.cpp \
   HID/xboxController.cpp \
   main.cpp \
   optionsManager.cpp \
   Scene/crosshairAsset.cpp \
   Scene/debuggingRayAsset.cpp \
   Scene/gridAsset.cpp \
   Scene/lineAsset.cpp \
   Scene/sceneAsset.cpp \
   Scene/visualizationAsset.cpp \
   Viewport/camera.cpp \
   Viewport/canvasContextMenu.cpp \
   Viewport/glCanvas.cpp \
   Viewport/graphicsDevice.cpp \
   Visualizations/squarifiedTreemap.cpp \
   Visualizations/visualization.cpp \
   Windows/aboutDialog.cpp \
   Windows/mainWindow.cpp \
   Windows/breakdownDialog.cpp \
   Windows/scanBreakdownModel.cpp

HEADERS  += \
   constants.h \
   controller.h \
   DataStructs/block.h \
   DataStructs/fileInfo.h \
   DataStructs/light.h \
   DataStructs/vizNode.h \
   DataStructs/driveScanningParameters.h \
   DataStructs/precisePoint.h \
   DataStructs/scanningprogress.hpp \
   DriveScanner/driveScanner.h \
   DriveScanner/scanningWorker.h \
   HID/keyboardManager.h \
   HID/xboxController.h \
   optionsManager.h \
   Scene/crosshairAsset.h \
   Scene/debuggingRayAsset.h \
   Scene/gridAsset.h \
   Scene/lineAsset.h \
   Scene/sceneAsset.h \
   Scene/visualizationAsset.h \
   ThirdParty/stopwatch.hpp \
   ThirdParty/ThreadSafeQueue.hpp \
   ThirdParty/Tree.hpp \
   Utilities/colorGradient.hpp \
   Utilities/scopeExit.hpp \
   Viewport/camera.h \
   Viewport/canvasContextMenu.h \
   Viewport/glCanvas.h \
   Viewport/graphicsDevice.h \
   Visualizations/squarifiedTreemap.h \
   Visualizations/visualization.h \
   Windows/aboutDialog.h \
   Windows/mainWindow.h \
   Windows/breakdownDialog.h \
   Windows/scanBreakdownModel.h

FORMS    += \
   Windows/mainWindow.ui \
   Windows/aboutDialog.ui \
    Windows/breakdownDialog.ui

INCLUDEPATH += ../../boost_1_60_0

DISTFILES += \
   Shaders/visualizationFragmentShader.frag \
   Shaders/visualizationVertexShader.vert \
   Shaders/simpleLineFragmentShader.frag \
   Shaders/simpleLineVertexShader.vert

RESOURCES += \
   resources.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../boost_1_60_0/stage/lib/ -lboost_filesystem-vc140-mt-1_60
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../boost_1_60_0/stage/lib/ -lboost_filesystem-vc140-mt-gd-1_60

INCLUDEPATH += $$PWD/../../boost_1_60_0/stage
DEPENDPATH += $$PWD/../../boost_1_60_0/stage

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../boost_1_60_0/stage/lib/libboost_filesystem-vc140-mt-1_60.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../boost_1_60_0/stage/lib/libboost_filesystem-vc140-mt-gd-1_60.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../boost_1_60_0/stage/lib/boost_filesystem-vc140-mt-1_60.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../boost_1_60_0/stage/lib/boost_filesystem-vc140-mt-gd-1_60.lib

win32:
   LIBS += -lXInput9_1_0
   LIBS += -lShell32
   LIBS += -lOle32

CONFIG(release, debug|release) {
   library_files.path += $$OUT_PWD/release
   library_files.files += $$PWD/../../boost_1_60_0/stage/lib/boost_filesystem-vc140-mt-1_60*
   library_files.files += $$PWD/../../boost_1_60_0/stage/lib/boost_system-vc140-mt-1_60*
   library_files.files += $$(QTDIR)/bin/Qt53DCore.dll
   library_files.files += $$(QTDIR)/bin/Qt5Widgets.dll
   library_files.files += $$(QTDIR)/bin/Qt5Gui.dll
   library_files.files += $$(QTDIR)/bin/Qt5Core.dll

   plugin_files.path += $$OUT_PWD/release/platforms
   plugin_files.files += $$(QTDIR)/plugins/platforms/qwindows.dll
}
CONFIG(debug, debug|release) {
   library_files.path += $$OUT_PWD/debug
   library_files.files += $$PWD/../../boost_1_60_0/stage/lib/boost_filesystem-vc140-mt-gd-1_60*
   library_files.files += $$PWD/../../boost_1_60_0/stage/lib/boost_system-vc140-mt-gd-1_60*
   library_files.files += $$(QTDIR)/bin/Qt53DCore.dll
   library_files.files += $$(QTDIR)/bin/Qt5Widgets.dll
   library_files.files += $$(QTDIR)/bin/Qt5Gui.dll
   library_files.files += $$(QTDIR)/bin/Qt5Core.dll

   plugin_files.path += $$OUT_PWD/release/platforms
   plugin_files.files += $$(QTDIR)/plugins/platforms/qwindows.dll
}

INSTALLS += library_files
INSTALLS += plugin_files
