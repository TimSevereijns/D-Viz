#include "light.h"

Light::Light()
   : position(QVector3D{0, 0, 0}),
     intensity(QVector3D{1, 1, 1}),
     attenuation(0.75f),
     ambientCoefficient(0.01f)
{
}

Light::Light(const QVector3D& lightPosition,
             const QVector3D& lightIntensity,
             const float lightAttenuation,
             const float lightAmbientCoefficient)
   : position(lightPosition),
     intensity(lightIntensity),
     attenuation(lightAttenuation),
     ambientCoefficient(lightAmbientCoefficient)
{
}

