#include "cameraTests.h"

#include <Viewport/camera.h>

void CameraTests::initTestCase()
{
}

void CameraTests::cleanupTestCase()
{
}

void CameraTests::SettingPosition()
{
    Camera camera;

    const QVector3D position{ 100, 100, 100 };
    camera.SetPosition(position);

    QCOMPARE(camera.GetPosition(), position);
}

void CameraTests::OffsetPosition()
{
    Camera camera;

    const QVector3D position{ 100, 100, 100 };
    camera.SetPosition(position);

    camera.OffsetPosition({ 10, 10, 10 });

    const auto expectedPosition = QVector3D{ 110, 110, 110 };
    QCOMPARE(camera.GetPosition(), expectedPosition);
}

void CameraTests::ForwardIsOppositeOfBackward()
{
    Camera camera;

    const QVector3D position{ 0, 0, 0 };
    camera.SetPosition(position);

    camera.LookAt(QVector3D{ 100, 100, 100 });

    const auto forwards = camera.Forward();
    QCOMPARE(forwards, -camera.Backward());
}

void CameraTests::LeftIsOppositeOfRight()
{
    Camera camera;

    const QVector3D position{ 0, 0, 0 };
    camera.SetPosition(position);

    camera.LookAt(QVector3D{ -100, -100, -100 });

    const auto left = camera.Left();
    QCOMPARE(left, -camera.Right());
}

void CameraTests::UpIsOppositeOfDown()
{
    Camera camera;

    const QVector3D position{ 0, 0, 0 };
    camera.SetPosition(position);

    camera.LookAt(QVector3D{ -100, 100, -100 });

    const auto up = camera.Up();
    QCOMPARE(up, -camera.Down());
}

void CameraTests::PointIsInFrontOfCamera()
{
    Camera camera;

    const QVector3D position{ 100, 100, 100 };
    camera.SetPosition(position);

    camera.LookAt(QVector3D{ 200, 100, 100 });

    QVERIFY(camera.IsPointInFrontOfCamera(QVector3D{ 128, 100, 100 }));
}

void CameraTests::PointIsNotInFrontOfCamera()
{
    Camera camera;

    const QVector3D position{ 100, 100, 100 };
    camera.SetPosition(position);

    camera.LookAt(QVector3D{ 99, 100, 100 });

    QCOMPARE(camera.IsPointInFrontOfCamera(QVector3D{ 128, 100, 100 }), false);
}

void CameraTests::PickingRay()
{
    Camera camera;

    const QVector3D position{ 100, 100, 100 };
    camera.SetPosition(position);

    const QRect viewport{ 0, 0, 100, 100 };
    camera.SetViewport(viewport);

    camera.SetNearPlane(1.0f);
    camera.SetFarPlane(1000.0f);

    const auto ray = camera.ShootRayIntoScene(viewport.center());

    const auto expectedOrigin = QVector3D{ 99.9917f, 100.008f, 99.0f };
    QCOMPARE(ray.Origin().x(), expectedOrigin.x());
    QCOMPARE(ray.Origin().y(), expectedOrigin.y());
    QCOMPARE(ray.Origin().z(), expectedOrigin.z());
}

REGISTER_TEST(CameraTests);
