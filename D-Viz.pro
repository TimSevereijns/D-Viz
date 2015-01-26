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
# TS: The following was auto-created by adding boost as a static lib.

unix|win32: LIBS += -L$$PWD/../../boost_1_57_0/stage/lib/ -llibboost_filesystem-vc120-mt-gd-1_57

INCLUDEPATH += $$PWD/../../boost_1_57_0/stage
DEPENDPATH += $$PWD/../../boost_1_57_0/stage

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../boost_1_57_0/stage/lib/libboost_filesystem-vc120-mt-gd-1_57.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../boost_1_57_0/stage/lib/liblibboost_filesystem-vc120-mt-gd-1_57.a

#
#-------------------------------------------------
