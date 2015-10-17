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
    glCanvas.cpp \
    camera.cpp \
    keyboardManager.cpp \
    Visualizations/sliceAndDiceTreemap.cpp \
    Visualizations/visualization.cpp \
    Visualizations/squarifiedTreemap.cpp \
    xboxController.cpp \
    optionsManager.cpp \
    Scene/visualizationAsset.cpp \
    Scene/sceneAsset.cpp \
    Scene/gridAsset.cpp \
    Scene/graphicsDevice.cpp \
    DriveScanner/scanningWorker.cpp \
    DataStructs/block.cpp \
    DataStructs/vizNode.cpp \
    DataStructs/fileInfo.cpp \
    DataStructs/light.cpp \
    DataStructs/doublePoint3d.cpp \
    DriveScanner/driveScanner.cpp

HEADERS  += mainwindow.h \
    tree.h \
    glCanvas.h \
    camera.h \
    keyboardManager.h \
    Visualizations/visualization.h \
    Visualizations/sliceAndDiceTreemap.h \
    Visualizations/squarifiedTreemap.h \
    xboxController.h \
    xInput.h \
    optionsManager.h \
    Scene/visualizationAsset.h \
    Scene/sceneAsset.h \
    Scene/gridAsset.h \
    Scene/graphicsDevice.h \
    DriveScanner/scanningWorker.h \
    DataStructs/block.h \
    DataStructs/vizNode.h \
    DataStructs/fileInfo.h \
    DataStructs/light.h \
    DriveScanner/driveScanner.h \
    DataStructs/doublePoint3d.h \
    Utilities/scopeExit.hpp

FORMS    += mainwindow.ui

INCLUDEPATH += ../boost_1_57_0

DISTFILES += \
    Shaders/originMarkerVertexShader.vert \
    Shaders/originMarkerFragmentShader.frag \
    Shaders/visualizationFragmentShader.frag \
    Shaders/visualizationVertexShader.vert

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
