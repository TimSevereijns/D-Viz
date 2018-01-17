#version 330 core

uniform vec3 cameraPosition;

uniform float materialShininess;

const int CASCADE_COUNT = 3;
uniform sampler2D shadowMaps[CASCADE_COUNT];

uniform struct Light
{
   vec3 position;
   vec3 intensity;
   float ambientCoefficient;
   float attenuation;
} allLights[5];

uniform float cascadeBounds[CASCADE_COUNT];

in vec3 vertexPosition;
in vec3 vertexColor;
in vec3 vertexNormal;

in vec4 shadowCoordinates[CASCADE_COUNT];

in float clipSpaceZ;

out vec4 finalPixelColor;

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

float ComputeShadowAttenuation(
   int cascadeIndex,
   vec4 shadowCoordinate)
{
   // @note Perspective division is automatically applied to `gl_position`. Coordinates passed along
   // manually, however, will need the division applied:
   vec3 projectionCoordinates = shadowCoordinate.xyz / shadowCoordinate.w;

   vec2 uvCoordinates;
   uvCoordinates.x = 0.5 * projectionCoordinates.x + 0.5;
   uvCoordinates.y = 0.5 * projectionCoordinates.y + 0.5;

   float z = 0.5 * projectionCoordinates.z + 0.5;

   float depth = texture(shadowMaps[cascadeIndex], uvCoordinates).x;

   // @note The bias being subtracted from the distance between the fragment and the light source
   // is to compensate for an artifact known as shadow acne; however, this bias will introduce yet
   // another artifact: Peter-panning. This new artifact can be compensated for with front-face
   // culling, which is implemented in the C++ code.
   if (depth < z - 0.00005)
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
   vec3 cascadeIndicator = vec3(0.0, 0.00, 0.0);

   float shadowFactor = 1.0;

   for (int index = 0; index < CASCADE_COUNT; ++index)
   {
      if (clipSpaceZ <= cascadeBounds[index])
      {
         shadowFactor = ComputeShadowAttenuation(index, shadowCoordinates[index]);

         if (index == 0)
         {
            cascadeIndicator = vec3(0.01, 0.0, 0.0);
         }
         else if (index == 1)
         {
            cascadeIndicator = vec3(0.0, 0.01, 0.0);
         }
         else if (index == 2)
         {
            cascadeIndicator = vec3(0.0, 0.0, 0.01);
         }

         break;
      }
   }

   vec3 lightFactor = ComputeLightContribution(
      allLights[1],
      vertexColor,
      vertexNormal,
      vertexPosition.xyz,
      fragmentToCamera,
      /* includeAmbient = */ false);

   vec3 fragmentColor = lightFactor * shadowFactor;// + cascadeIndicator;

   vec3 gammaCorrection = vec3(1.0f / 2.2f);
   finalPixelColor = vec4(pow(fragmentColor, gammaCorrection), 1);
}
