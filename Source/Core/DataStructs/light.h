#ifndef LIGHT_H
#define LIGHT_H

#include <QVector3D>
#include <QVector>

/**
 * @brief The Light struct represents a single point light.
 */
struct Light
{
    QVector3D position{ 0.0f, 0.0f, 0.0f };  ///< The position of the light in 3D space.
    QVector3D intensity{ 1.0f, 1.0f, 1.0f }; ///< The light color.
    float attenuation{ 0.75f };        ///< The factor with which the light falls off with distance.
    float ambientCoefficient{ 0.01f }; ///< The light's contribution to ambient.

    Light() = default;

    Light(
        const QVector3D& lightPosition, const QVector3D& lightIntensity,
        const float lightAttenuation, const float lightAmbientCoefficient);

    explicit Light(const QVector3D& lightPosition);
};

#endif // LIGHT_H
