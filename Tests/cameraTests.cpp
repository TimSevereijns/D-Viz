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
    const QVector3D position{ 100, 100, 100 };

    Camera camera;
    camera.SetPosition(position);

    QCOMPARE(camera.GetPosition(), position);
}

void CameraTests::OffsetPosition()
{
    const QVector3D position{ 100, 100, 100 };

    Camera camera;
    camera.SetPosition(position);
    camera.OffsetPosition({ 10, 10, 10 });

    const auto expectedPosition = QVector3D{ 110, 110, 110 };
    QCOMPARE(camera.GetPosition(), expectedPosition);
}

void CameraTests::PickingRay()
{
    const QVector3D position{ 100, 100, 100 };

    Camera camera;
    camera.SetPosition(position);

    QRect viewport{ 0, 0, 100, 100 };
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
