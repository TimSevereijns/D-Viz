#ifndef CAMERATESTS_H
#define CAMERATESTS_H

#include <QTest>

#include "Utilities/multiTestHarness.h"

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
     * @brief Verify that the forward vector is the opposite of the backwards vector.
     */
    void ForwardIsOppositeOfBackward();

    /**
     * @brief Verify that the left vector is the opposite of the right vector.
     */
    void LeftIsOppositeOfRight();

    /**
     * @brief Verify that the up vector is the opposite of the down vector.
     */
    void UpIsOppositeOfDown();

    /**
     * @brief Verify that a point in front of the camera's near plan is indeed detected as being in
     * front of the camera.
     */
    void PointIsInFrontOfCamera();

    /**
     * @brief Verify that a point not in front of the camera's near plan is indeed detected as not
     * being in front of the camera.
     */
    void PointIsNotInFrontOfCamera();

    /**
     * @brief Verify that a ray can be emitted from the camera.
     */
    void PickingRay();
};

#endif // CAMERATESTS_H
