#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T19:01:44
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets 3dcore

TARGET = D-Viz
TEMPLATE = app
CONFIG += c++11 console

SOURCES += main.cpp\
    mainwindow.cpp \
    optionsManager.cpp \
    DataStructs/block.cpp \
    DataStructs/fileInfo.cpp \
    DataStructs/light.cpp \
    DataStructs/vizNode.cpp \
    DataStructs/doublePoint3d.cpp \
    DataStructs/driveScanningParameters.cpp \
    DriveScanner/driveScanner.cpp \
    DriveScanner/scanningWorker.cpp \
    HID/keyboardManager.cpp \
    HID/xboxController.cpp \
    Scene/debuggingRayAsset.cpp \
    Scene/gridAsset.cpp \
    Scene/lineAsset.cpp \
    Scene/sceneAsset.cpp \
    Scene/visualizationAsset.cpp \
    Viewport/camera.cpp \
    Viewport/glCanvas.cpp \
    Viewport/graphicsDevice.cpp \
    Visualizations/squarifiedTreemap.cpp \
    Visualizations/visualization.cpp \
    Scene/nodeSelectionCrosshair.cpp

HEADERS  += mainwindow.h \
    optionsManager.h \
    DataStructs/block.h \
    DataStructs/fileInfo.h \
    DataStructs/light.h \
    DataStructs/vizNode.h \
    DataStructs/doublePoint3d.h \
    DataStructs/driveScanningParameters.h \
    DriveScanner/driveScanner.h \
    DriveScanner/scanningWorker.h \
    HID/keyboardManager.h \
    HID/xboxController.h \
    HID/xInput.h \
    Scene/debuggingRayAsset.h \
    Scene/gridAsset.h \
    Scene/lineAsset.h \
    Scene/sceneAsset.h \
    Scene/visualizationAsset.h \
    ThirdParty/Tree.hpp \
    Utilities/scopeExit.hpp \
    Viewport/camera.h \
    Viewport/glCanvas.h \
    Viewport/graphicsDevice.h \
    Visualizations/squarifiedTreemap.h \
    Visualizations/visualization.h \
    Utilities/stopwatch.hpp \
    constants.h \
    Scene/nodeSelectionCrosshair.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../boost_1_57_0

DISTFILES += \
    Shaders/visualizationFragmentShader.frag \
    Shaders/visualizationVertexShader.vert \
    Shaders/simpleLineFragmentShader.frag \
    Shaders/simpleLineVertexShader.vert \
    Resources/XInput1_4.dll

RESOURCES += \
    resources.qrc

# TS: The following was auto-generated by adding "libboost_system-vc120-mt-1_57.lib" as a static lib.
#     I left the -d option checked, even though the debug files do not end in 'd', and then modified
#     the following code myself. Boost uses "...-gd-..." to denote debug files.

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../boost_1_57_0/stage/lib/ -lboost_filesystem-vc120-mt-1_57
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../boost_1_57_0/stage/lib/ -lboost_filesystem-vc120-mt-gd-1_57
else:unix: LIBS += -L$$PWD/../boost_1_57_0/stage/lib/ -lboost_filesystem-vc120-mt-1_57

INCLUDEPATH += $$PWD/../boost_1_57_0/stage
DEPENDPATH += $$PWD/../boost_1_57_0/stage

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../boost_1_57_0/stage/lib/libboost_filesystem-vc120-mt-1_57.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../boost_1_57_0/stage/lib/libboost_filesystem-vc120-mt-gd-1_57.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../boost_1_57_0/stage/lib/boost_filesystem-vc120-mt-1_57.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../boost_1_57_0/stage/lib/boost_filesystem-vc120-mt-gd-1_57.lib
else:unix: PRE_TARGETDEPS += $$PWD/../boost_1_57_0/stage/lib/libboost_filesystem-vc120-mt-1_57.a

win32: LIBS += -lXInput
