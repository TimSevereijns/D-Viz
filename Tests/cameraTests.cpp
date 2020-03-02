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

    constexpr QVector3D position{ 100, 100, 100 };
    camera.SetPosition(position);

    QCOMPARE(camera.GetPosition(), position);
}

void CameraTests::OffsetPosition()
{
    Camera camera;

    camera.SetPosition({ 100, 100, 100 });
    camera.OffsetPosition({ 10, 10, 10 });

    constexpr QVector3D expectedPosition{ 110, 110, 110 };
    QCOMPARE(camera.GetPosition(), expectedPosition);
}

void CameraTests::ForwardIsOppositeOfBackward()
{
    Camera camera;

    camera.SetPosition({ 0, 0, 0 });
    camera.LookAt({ 100, 100, 100 });

    const auto forwards = camera.Forward();
    QCOMPARE(forwards, -camera.Backward());
}

void CameraTests::LeftIsOppositeOfRight()
{
    Camera camera;

    camera.SetPosition({ 0, 0, 0 });
    camera.LookAt({ -100, -100, -100 });

    const auto left = camera.Left();
    QCOMPARE(left, -camera.Right());
}

void CameraTests::UpIsOppositeOfDown()
{
    Camera camera;

    camera.SetPosition({ 0, 0, 0 });
    camera.LookAt({ -100, 100, -100 });

    const auto up = camera.Up();
    QCOMPARE(up, -camera.Down());
}

void CameraTests::PointIsInFrontOfCamera()
{
    Camera camera;

    camera.SetPosition({ 100, 100, 100 });
    camera.LookAt({ 200, 100, 100 });

    QVERIFY(camera.IsPointInFrontOfCamera({ 128, 100, 100 }));
}

void CameraTests::PointIsNotInFrontOfCamera()
{
    Camera camera;

    camera.SetPosition({ 100, 100, 100 });
    camera.LookAt(QVector3D{ 99, 100, 100 });

    QCOMPARE(camera.IsPointInFrontOfCamera({ 128, 100, 100 }), false);
}

void CameraTests::PickingRay()
{
    Camera camera;

    camera.SetPosition({ 100, 100, 100 });

    constexpr QRect viewport{ 0, 0, 100, 100 };
    camera.SetViewport(viewport);

    camera.SetNearPlane(1.0f);
    camera.SetFarPlane(1000.0f);

    const auto ray = camera.ShootRayIntoScene(viewport.center());

    constexpr QVector3D expectedOrigin{ 99.9917f, 100.008f, 99.0f };
    QCOMPARE(ray.Origin().x(), expectedOrigin.x());
    QCOMPARE(ray.Origin().y(), expectedOrigin.y());
    QCOMPARE(ray.Origin().z(), expectedOrigin.z());
}

REGISTER_TEST(CameraTests);
