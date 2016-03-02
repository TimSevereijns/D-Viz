#ifndef LIGHT_H
#define LIGHT_H

#include <QVector>
#include <QVector3D>

/**
 * @brief The Light struct represents a single point light.
 */
struct Light
{
   QVector3D position;              ///< The position of the light in 3D space.
   QVector3D intensity;             ///< The intensity of the RGB values, i.e., the light color.
   float attenuation;               ///< The factor with which the light falls off with distance.
   float ambientCoefficient;        ///< The light's contribution to ambient.

   explicit Light();

   explicit Light(
      const QVector3D& lightPosition,
      const QVector3D& lightIntensity,
      const float lightAttenuation,
      const float lightAmbientCoefficient);

   explicit Light(const QVector3D& lightPosition);
};

#endif // LIGHT_H
