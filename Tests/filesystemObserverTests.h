#ifndef FILESYSTEMOBSERVERTESTS_H
#define FILESYSTEMOBSERVERTESTS_H

#include <QTest>

#include "multiTestHarness.h"

class FilesystemObserverTests : public QObject
{
    Q_OBJECT

  private slots:

    /**
     * @brief This preamble is run only once for the entire class. All setup work should be done
     * here.
     */
    void initTestCase();

    /**
     * @brief Clean up code for the entire class; called once.
     */
    void cleanupTestCase();

    /**
     * @brief Monitors an actual directory on disk as the files it contains are deleted.
     */
    void MonitorDeletions();
};

#endif // FILESYSTEMOBSERVERTESTS_H
