#include "View/Scene/light.h"

Light::Light(
    const QVector3D& lightPosition, const QVector3D& lightIntensity, const float lightAttenuation,
    const float lightAmbientCoefficient) noexcept
    : position{ lightPosition },
      intensity{ lightIntensity },
      attenuation{ lightAttenuation },
      ambientCoefficient{ lightAmbientCoefficient }
{
}

Light::Light(const QVector3D& lightPosition) noexcept
    : position{ lightPosition },
      intensity{ QVector3D{ 0.1f, 0.1f, 0.1f } },
      attenuation{ 0.1f },
      ambientCoefficient{ 0.01f }
{
}
