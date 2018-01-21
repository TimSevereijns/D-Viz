#version 330 core

uniform vec3 cameraPosition;

uniform float materialShininess;

uniform bool shouldShowCascadeSplits;
uniform bool shouldShowShadows;

const vec3 sunPosition = vec3(0.0, 200.0, 0.0);

const int pcfLevel = 2;

const int CASCADE_COUNT = 4;
uniform sampler2D shadowMaps[CASCADE_COUNT];

const int LIGHT_COUNT = 5;
uniform struct Light
{
   vec3 position;
   vec3 intensity;
   float ambientCoefficient;
   float attenuation;
} allLights[LIGHT_COUNT];

uniform float cascadeBounds[CASCADE_COUNT];

const float shadowBias[CASCADE_COUNT] = float[CASCADE_COUNT](0.00002, 0.00004, 0.00016, 0.00064);

// @note Use this one when PCF is enabled:
//const float shadowBias[CASCADE_COUNT] = float[CASCADE_COUNT](0.0001, 0.0002, 0.0008, 0.0024);

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
   // The bias used to prevent shadow acne, causes a slight separation of the shadow from the back
   // face of a block. A common fix this this problem is to cull front faces when rendering the
   // shadow map. This approach, however, doesn't tend to do well with thing geometries, so instead,
   // we'll simply consider all fragments that make up a back face of a block to be in shadow if
   // its normal points away from the "sun."
   vec3 surfaceToLight = normalize(sunPosition - vec3(vertexPosition));
   if (dot(surfaceToLight, vertexNormal) < 0)
   {
      return 0.5;
   }

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
   // another artifact: Peter-panning.
   return depth < z - shadowBias[cascadeIndex] ? 0.5 : 1.0;


   // Percent Closer Filtering:
//   float shadow = 0.0;
//   vec2 texelSize = 1.0 / textureSize(shadowMaps[cascadeIndex], 0);
//   for (int x = -pcfLevel; x <= pcfLevel; ++x)
//   {
//       for (int y = -pcfLevel; y <= pcfLevel; ++y)
//       {
//           float depth = texture(shadowMaps[cascadeIndex], uvCoordinates.xy + vec2(x, y) * texelSize).x;
//           shadow += depth < z - shadowBias[cascadeIndex] ? 0.5 : 1.0;
//       }
//   }

//   shadow /= ((pcfLevel * 2) + 1) * ((pcfLevel * 2) + 1);
//   return shadow;
}

void main(void)
{
   vec3 fragmentToCamera = normalize(cameraPosition - vec3(vertexPosition));
   vec3 cascadeIndicator = vec3(0.0, 0.00, 0.0);

   float shadowFactor = 1.0;

   if (shouldShowShadows)
   {
      for (int index = 0; index < CASCADE_COUNT; ++index)
      {
         if (clipSpaceZ <= cascadeBounds[index])
         {
            shadowFactor = ComputeShadowAttenuation(index, shadowCoordinates[index]);

            if (index == 0)
            {
               cascadeIndicator = vec3(0.1, 0.0, 0.0);
            }
            else if (index == 1)
            {
               cascadeIndicator = vec3(0.0, 0.1, 0.0);
            }
            else if (index == 2)
            {
               cascadeIndicator = vec3(0.0, 0.0, 0.1);
            }
            else if (index == 3)
            {
               cascadeIndicator = vec3(0.1, 0.1, 0.1);
            }

            break;
         }
      }
   }

   vec3 lightFactor = vec3(0.0, 0.0, 0.0);
   for (int index = 0; index < LIGHT_COUNT; ++index)
   {
      lightFactor += ComputeLightContribution(
         allLights[index],
         vertexColor,
         vertexNormal,
         vertexPosition.xyz,
         fragmentToCamera,
         /* includeAmbient = */ false);
   }

   vec3 fragmentColor = lightFactor * shadowFactor;
   if (shouldShowCascadeSplits)
   {
      fragmentColor += cascadeIndicator;
   }

   vec3 gammaCorrection = vec3(1.0f / 2.2f);
   finalPixelColor = vec4(pow(fragmentColor, gammaCorrection), 1);
}
