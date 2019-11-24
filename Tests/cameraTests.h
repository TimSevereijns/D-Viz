#ifndef CAMERATESTS_H
#define CAMERATESTS_H

#include <QTest>

#include "multiTestHarness.h"

class CameraTests : public QObject
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
     * @brief The camera's position should be easily set and retrieved.
     */
    void SettingPosition();

    /**
     * @brief Verify that the camera can be translated via a simple offset.
     */
    void OffsetPosition();

    /**
     * @brief Verify that a ray can be emitted from the camera.
     */
    void PickingRay();
};

#endif // CAMERATESTS_H
