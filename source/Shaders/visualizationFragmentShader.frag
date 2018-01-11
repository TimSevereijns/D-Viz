#version 330 core

uniform vec3 cameraPosition;

uniform float materialShininess;

uniform sampler2D shadowMap;

uniform struct Light
{
   vec3 position;
   vec3 intensity;
   float ambientCoefficient;
   float attenuation;
} allLights[5];

in vec3 vertexPosition;
in vec3 vertexColor;
in vec3 vertexNormal;

in vec4 shadowCoordinate;

out vec4 pixelColor;

vec3 ComputeLightContribution(
   Light light,
   vec3 surfaceColor,
   vec3 normal,
   vec3 surfacePosition,
   vec3 surfaceToCamera,
   bool includeAmbient)
{
   vec3 surfaceToLight = normalize(light.position - surfacePosition);

   // Ambient:
   vec3 ambient = light.ambientCoefficient * surfaceColor * light.intensity;

   // Diffuse:
   float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
   vec3 diffuse = diffuseCoefficient * surfaceColor * light.intensity;

   // Specular:
   float specularCoefficient = 0.0;
   if (diffuseCoefficient > 0.0)
   {
      specularCoefficient = pow(
         max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))),
         materialShininess);
   }
   vec3 materialSpecularColor = vec3(0.0f, 0.0f, 0.0f);
   vec3 specular = specularCoefficient * materialSpecularColor * light.intensity;

   // Attenuation:
   float distanceToLight = length(light.position - surfacePosition);
   float attenuation = 1.0 / (1.0 + light.attenuation * distanceToLight);

   // Linear color (before gamma correction):
   vec3 linearColor = (includeAmbient ? ambient : vec3(0.0f));
   linearColor += attenuation * (diffuse + specular);

   return linearColor;
}

float ComputeShadowAttenuation()
{
   vec3 projectionCoordinates = shadowCoordinate.xyz / shadowCoordinate.w;

   vec2 UVCoordinates;
   UVCoordinates.x = 0.5 * projectionCoordinates.x + 0.5;
   UVCoordinates.y = 0.5 * projectionCoordinates.y + 0.5;

   float z = 0.5 * projectionCoordinates.z + 0.5;

   float Depth = texture(shadowMap, UVCoordinates).x;

   // @note The bias that we subtracting from the distance between the fragment and the light source
   // is to compensate for shadow acne; however, this bias will introduce another artifact: Peter-
   // panning. This can be compensated for with front-face culling.
   if (Depth < z - 0.00005)
   {
      return 0.5;
   }
   else
   {
      return 1.0;
   }
}

void main(void)
{
   vec3 fragmentToCamera = normalize(cameraPosition - vec3(vertexPosition));

   vec3 linearColor = vec3(0.0f);

   // Calculate the contribution of the shadow casting light:
   linearColor += ComputeShadowAttenuation() *
      ComputeLightContribution(
         allLights[1],
         vertexColor,
         vertexNormal,
         vertexPosition.xyz,
         fragmentToCamera,
         /* includeAmbient = */ false);

   // Gamma correction:
   vec3 gamma = vec3(1.0f / 2.2f);

   // Final pixel color:
   pixelColor = vec4(pow(linearColor, gamma), 1);
}
