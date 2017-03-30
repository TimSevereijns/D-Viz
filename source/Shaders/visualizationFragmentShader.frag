#version 450 core

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
   vec3 surfaceToCamera)
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
   vec3 linearColor = ambient + attenuation * (diffuse + specular);
   return linearColor;
}

float near_plane = 1.0f;
float far_plane = 2000.0f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to Normalized Device Coordinates (NDC)
    return (2.0 * near_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
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

   float bias = 0.000001 * tan(acos(cosTheta)); // cosTheta is dot( n,l ), clamped between 0 and 1
   bias = clamp(bias, 0.0, 0.000001);

   //float bias = 0.00000001;
   float shadow = distanceToFragment - bias < distanceToOccluder  ? 1.0 : 0.2;

   // Percent Closer Filtering:
//   float shadow = 0.0;
//   vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//   for(int x = -3; x <= 3; ++x)
//   {
//       for(int y = -3; y <= 3; ++y)
//       {
//           float depth = texture(shadowMap, projectionCoordinates.xy + vec2(x, y) * texelSize).r;
//           shadow += distanceToFragment - bias < depth  ? 1.0 : 0.2;
//       }
//   }

//   shadow /= 49.0;

   // Keep the shadow at 0.2 when outside the far_plane region of the light's frustum.
   if (projectionCoordinates.z > 1.0)
   {
      return 0.2;
   }

   return shadow;
}

void main(void)
{
   vec3 fragmentToCamera = normalize(cameraPosition - vec3(vertexPosition));

   // Calculate the contribution of each light:
   vec3 linearColor = vec3(0.0f, 0.0f, 0.0f);
   //for (int i = 0; i < 5; ++i)
   {
      linearColor += ComputeLightContribution(
         allLights[1],
         vertexColor,
         vertexNormal,
         vertexPosition.xyz,
         fragmentToCamera);
   }

   // Gamma correction:
   vec3 gamma = vec3(1.0f / 2.2f);

   // Final pixel color:
   pixelColor = vec4(pow(linearColor, gamma), 1) * ComputeShadowAttenuation();
}
