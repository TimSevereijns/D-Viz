#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T19:01:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = D-Viz
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    diskScanner.cpp

HEADERS  += mainwindow.h \
    tree.h \
    diskScanner.h

FORMS    += mainwindow.ui

INCLUDEPATH += C:\excluded\Misc\Qt\boost_1_57_0

#-------------------------------------------------
# INCLUDE THIS FOR DEBUG MODE:
# TS: The following was auto-created by adding libboost_filesystem-vc120-mt-gd-1_57.lib as a static lib.
#unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_filesystem-vc120-mt-gd-1_57

#INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
#DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_filesystem-vc120-mt-gd-1_57.lib
#else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_filesystem-vc120-mt-gd-1_57.a
#
#-------------------------------------------------

#-------------------------------------------------
# INCLUDE THIS FOR DEBUG MODE:
# TS: The following was auto-created by adding libboost_system-vc120-mt-gd-1_57.lib as a static lib.
#unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_system-vc120-mt-gd-1_57

#INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
#DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_system-vc120-mt-gd-1_57.lib
#else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_system-vc120-mt-gd-1_57.a
#
#-------------------------------------------------

#-------------------------------------------------
# INCLUDE THIS FOR RELEASE MODE:
# TS: The following was auto-created by adding libboost_system-vc120-mt-1_57.lib as a static lib.
unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_filesystem-vc120-mt-1_57

INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_filesystem-vc120-mt-1_57.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_filesystem-vc120-mt-1_57.a
#
#-------------------------------------------------

#-------------------------------------------------
# INCLUDE THIS FOR RELEASE MODE:
# TS: The following was auto-created by adding libboost_system-vc120-mt-1_57.lib as a static lib.
unix|win32: LIBS += -L$$PWD/../../../boost_1_57_0/lib64-msvc-12.0/ -llibboost_system-vc120-mt-1_57

INCLUDEPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0
DEPENDPATH += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/libboost_system-vc120-mt-1_57.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../boost_1_57_0/lib64-msvc-12.0/liblibboost_system-vc120-mt-1_57.a
#
#-------------------------------------------------
