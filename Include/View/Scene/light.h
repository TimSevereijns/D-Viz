#ifndef LIGHT_H
#define LIGHT_H

#include <QVector3D>
#include <QVector>

/**
 * @brief Represents a single point light.
 */
class Light
{
  public:
    Light() noexcept = default;

    Light(
        const QVector3D& lightPosition, const QVector3D& lightIntensity,
        const float lightAttenuation, const float lightAmbientCoefficient) noexcept;

    explicit Light(const QVector3D& lightPosition) noexcept;

    QVector3D position{ 0.0f, 0.0f, 0.0f };
    QVector3D intensity{ 1.0f, 1.0f, 1.0f }; //< Think color

    float attenuation{ 0.75f };
    float ambientCoefficient{ 0.01f }; //< The light's contribution to ambient.
};

#endif // LIGHT_H
