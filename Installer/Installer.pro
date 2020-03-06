TEMPLATE = aux

QT_INSTALL_FRAMEWORK_PATH = C:/Qt/Tools/QtInstallerFramework/3.2
QT_WIN_DEPLOY_PATH = C:/Qt/5.14.1/msvc2017_64

create_package.commands = \
    $$QT_WIN_DEPLOY_PATH/bin/windeployqt \
    --no-translations \
    $$PWD/packages/ics.component/data/D-Viz.exe

QMAKE_EXTRA_TARGETS += create_package
PRE_TARGETDEPS += create_package

DISTFILES += \
    config/config.xml \
    packages/ics.component/meta/package.xml \
    packages/ics.component/meta/installscript.qs

INSTALLER = d-viz-installer

INPUT = $$PWD/config/config.xml $$PWD/packages

d-viz-installer.input = INPUT
d-viz-installer.output = $$INSTALLER
d-viz-installer.CONFIG += target_predeps no_link combine
d-viz-installer.commands = \
    $$QT_INSTALL_FRAMEWORK_PATH/bin/binarycreator \
    --offline-only \
    -t $$QT_INSTALL_FRAMEWORK_PATH/bin/installerbase.exe \
    -c $$PWD/config/config.xml \
    -p $$PWD/packages \
    -v \
    C:\Users\Tim\Desktop\D-Viz-Installer.exe

QMAKE_EXTRA_COMPILERS += d-viz-installer
