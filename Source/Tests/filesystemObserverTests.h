#ifndef FILESYSTEMOBSERVERTESTS_H
#define FILESYSTEMOBSERVERTESTS_H

#include <QTest>

#include "multiTestHarness.h"

class FilesystemObserverTests : public QObject
{
    Q_OBJECT

  private slots:

    void initTestCase();

    void cleanupTestCase();

    void MonitorDeletions();
};

#endif // FILESYSTEMOBSERVERTESTS_H
