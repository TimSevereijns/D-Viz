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

float nearPlane = 1.0f;
float farPlane = 2000.0f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to Normalized Device Coordinates (NDC)
    return (2.0 * nearPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

float ComputeShadowAttenuation()
{
   // Perform perspective divide, and transform result to [0, 1] range:
   vec3 projectionCoordinates = shadowCoordinate.xyz / shadowCoordinate.w;
   projectionCoordinates = projectionCoordinates * 0.5 + 0.5;

   float distanceToOccluder = texture(shadowMap, projectionCoordinates.xy).r;
   float distanceToFragment = projectionCoordinates.z;

   vec3 fragmentToLight = normalize(allLights[1].position - vertexPosition.xyz);
   float cosTheta = dot(vertexNormal, fragmentToLight);

   float bias = 0.00000001;
   float shadow = distanceToFragment - bias < distanceToOccluder  ? 1.0 : 0.3;

   // Percent Closer Filtering:
//   float shadow = 0.0;
//   vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//   for (int x = -1; x <= 1; ++x)
//   {
//       for (int y = -1; y <= 1; ++y)
//       {
//           float depth = texture(shadowMap, projectionCoordinates.xy + vec2(x, y) * texelSize).r;
//           shadow += distanceToFragment - bias < depth  ? 1.0 : 0.0;
//       }
//   }
//   shadow /= 9.0;

   // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
   if (projectionCoordinates.z > 1.0)
   {
      return 0.0;
   }

   return shadow;
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

   // Calculate the contribution of the non-shadow casting lights:
//   for (int i = 2; i < 5; ++i)
//   {
//      linearColor += ComputeLightContribution(
//         allLights[i],
//         vertexColor,
//         vertexNormal,
//         vertexPosition.xyz,
//         fragmentToCamera,
//         /* includeAmbient = */ true);
//   }

   // Gamma correction:
   vec3 gamma = vec3(1.0f / 2.2f);

   // Final pixel color:
   pixelColor = vec4(pow(linearColor, gamma), 1);
}
