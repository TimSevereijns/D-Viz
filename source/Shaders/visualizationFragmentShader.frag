#version 450 core

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
   // perform perspective divide
   vec3 projCoords = shadowCoordinate.xyz / shadowCoordinate.w;
   // Transform to [0,1] range
   projCoords = projCoords * 0.5 + 0.5;
   // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
   float closestDepth = texture(shadowMap, projCoords.xy).r;
   // Get depth of current fragment from light's perspective
   float currentDepth = projCoords.z;
   // Check whether current frag pos is in shadow
   float shadow = (currentDepth - 0.005f) > closestDepth  ? 1.0 : 0.5;

   return shadow;
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

   //float depth = texture(shadowMap, gl_FragCoord.xy).r;
   //pixelColor = vec4(vec3(depth), 1.0);

   //pixelColor = vec4(vec3(ComputeShadowAttenuation()), 1.0);
}
