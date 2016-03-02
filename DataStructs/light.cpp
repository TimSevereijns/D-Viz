#include "light.h"

Light::Light()
   : position(QVector3D{0.0f, 0.0f, 0.0f}),
     intensity(QVector3D{1.0f, 1.0f, 1.0f}),
     attenuation(0.75f),
     ambientCoefficient(0.01f)
{
}

Light::Light(
   const QVector3D& lightPosition,
   const QVector3D& lightIntensity,
   const float lightAttenuation,
   const float lightAmbientCoefficient)
   : position(lightPosition),
     intensity(lightIntensity),
     attenuation(lightAttenuation),
     ambientCoefficient(lightAmbientCoefficient)
{
}

Light::Light(const QVector3D& lightPosition)
   : position(lightPosition),
     intensity(QVector3D{1.0f, 1.0f, 1.0f}),
     attenuation(0.75f),
     ambientCoefficient(0.01f)
{
}

