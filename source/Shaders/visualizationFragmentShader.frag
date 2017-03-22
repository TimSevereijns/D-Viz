#version 410 core

uniform vec3 cameraPosition;
uniform vec3 materialSpecularColor;

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
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

float ComputeShadowAttenuation()
{
   float shadowAcneBias = 0.005f;
   float distanceToNearestOccluder = LinearizeDepth(texture2D(shadowMap, shadowCoordinate.xy).r);
   float distanceToFragment = shadowCoordinate.z - shadowAcneBias;

   return (distanceToNearestOccluder < distanceToFragment) ? 0.5f : 1.0f;
}

void main(void)
{
   vec3 fragmentToCamera = normalize(cameraPosition - vec3(vertexPosition));

   // Calculate the contribution of each light:
   vec3 linearColor = vec3(0.0f, 0.0f, 0.0f);
//   for (int i = 0; i < 5; ++i)
//   {
      linearColor += ComputeLightContribution(
         allLights[1],
         vertexColor,
         vertexNormal,
         vertexPosition.xyz,
         fragmentToCamera);
   //}

   // Gamma correction:
   vec3 gamma = vec3(1.0f / 2.2f);

   // Final pixel color:
   pixelColor = vec4(pow(linearColor, gamma), 1) * ComputeShadowAttenuation();

   // DEBUG: This will allow for the visualization of the camera's depth buffer:
   float depth = texture2D(shadowMap, shadowCoordinate.xy).r;//LinearizeDepth(gl_FragCoord.z);
   depth = (depth == 0.0) ? 0.0 : 0.75;
   pixelColor = vec4(depth, depth, depth, 1.0);
}
