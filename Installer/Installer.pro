# Add "CONFIG+=BUILD_INSTALLER" as an argument to qmake to force this .pro file to be
# evaluated and run.
requires(BUILD_INSTALLER)

TEMPLATE = aux

win32: CONFIG(release, debug|release) {
    # Utility function to copy files from one location to another.
    defineTest(copyToDestDir) {
        files = $$1
        dir = $$2
        # Replace slashes in destination path for Windows
        win32:dir ~= s,/,\\,g

        for (file, files) {
            # Replace slashes in source path for Windows
            win32:file ~= s,/,\\,g

            QMAKE_POST_LINK += $$QMAKE_COPY \
                $$shell_quote($$file) \
                $$shell_quote($$dir) \
                $$escape_expand(\\n\\t)
        }

        export(QMAKE_POST_LINK)
    }

    FILES_TO_COPY += \
        $$PWD/../Output/Release/D-Viz.exe \
        $$PWD/../Output/Release/D-Viz.lib \
        $$PWD/../Output/Release/D-Viz.pdb

    copyToDestDir($$FILES_TO_COPY, $$PWD/packages/ics.component/data/)

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

    INSTALLER = offlineInstaller

    INPUT = $$PWD/config/config.xml $$PWD/packages

    offlineInstaller.input = INPUT
    offlineInstaller.output = $$INSTALLER
    offlineInstaller.CONFIG += target_predeps no_link combine
    offlineInstaller.commands = \
        $$QT_INSTALL_FRAMEWORK_PATH/bin/binarycreator \
        --offline-only \
        -t $$QT_INSTALL_FRAMEWORK_PATH/bin/installerbase.exe \
        -c $$PWD/config/config.xml \
        -p $$PWD/packages \
        -v \
        $$PWD/../Output/D-Viz-Installer.exe

    QMAKE_EXTRA_COMPILERS += offlineInstaller
}
