#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T19:01:44
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = D-Viz
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
    mainwindow.cpp \
    diskScanner.cpp \
    glCanvas.cpp \
    camera.cpp \
    keyboardManager.cpp \
    Visualizations/sliceAndDiceTreemap.cpp \
    Visualizations/visualization.cpp \
    Visualizations/squarifiedTreemap.cpp

HEADERS  += mainwindow.h \
    tree.h \
    diskScanner.h \
    glCanvas.h \
    camera.h \
    keyboardManager.h \
    Visualizations/visualization.h \
    Visualizations/sliceAndDiceTreemap.h \
    Visualizations/squarifiedTreemap.h

FORMS    += mainwindow.ui

#INCLUDEPATH += C:\excluded\Misc\Qt\boost_1_57_0
INCLUDEPATH += C:\excluded\Misc\D-Viz\boost_1_57_0

#-------------------------------------------------
# INCLUDE THIS FOR DEBUG MODE:
# TS: The following was auto-created by adding libboost_filesystem-vc120-mt-gd-1_57.lib as a static lib.
unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_filesystem-vc120-mt-gd-1_57

INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_filesystem-vc120-mt-gd-1_57.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_filesystem-vc120-mt-gd-1_57.a
#
#-------------------------------------------------

#-------------------------------------------------
# INCLUDE THIS FOR DEBUG MODE:
# TS: The following was auto-created by adding libboost_system-vc120-mt-gd-1_57.lib as a static lib.
unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_system-vc120-mt-gd-1_57

INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_system-vc120-mt-gd-1_57.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_system-vc120-mt-gd-1_57.a
#
#-------------------------------------------------

#-------------------------------------------------
# INCLUDE THIS FOR RELEASE MODE:
# TS: The following was auto-created by adding libboost_system-vc120-mt-1_57.lib as a static lib.
#unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_filesystem-vc120-mt-1_57

#INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
#DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_filesystem-vc120-mt-1_57.lib
#else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_filesystem-vc120-mt-1_57.a
#
#-------------------------------------------------

#-------------------------------------------------
# INCLUDE THIS FOR RELEASE MODE:
# TS: The following was auto-created by adding libboost_system-vc120-mt-1_57.lib as a static lib.
#unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_system-vc120-mt-1_57

#INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
#DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_system-vc120-mt-1_57.lib
#else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_system-vc120-mt-1_57.a
#
#-------------------------------------------------

DISTFILES += \
    Shaders/originMarkerVertexShader.vert \
    Shaders/originMarkerFragmentShader.frag \
    Shaders/visualizationFragmentShader.frag \
    Shaders/visualizationVertexShader.vert

RESOURCES += \
    resources.qrc
