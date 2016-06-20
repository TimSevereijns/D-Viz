#version 330

uniform mat4 model;

uniform vec3 cameraPosition;
uniform vec3 materialSpecularColor;

uniform float materialShininess;

uniform struct Light
{
   vec3 position;
   vec3 intensity;
   float ambientCoefficient;
   float attenuation;
} allLights[5];

in vec3 fragmentVertex;
in vec3 fragmentColor;
in vec3 fragmentNormal;

out vec4 pixelColor;

vec3 GetContributionOfLight(
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
      specularCoefficient = pow(max(0.0, dot(surfaceToCamera,
         reflect(-surfaceToLight, normal))), materialShininess);
   }
   vec3 specular = specularCoefficient * materialSpecularColor * light.intensity;

   // Attenuation:
   float distanceToLight = length(light.position - surfacePosition);
   float attenuation = 1.0 / (1.0 + light.attenuation * distanceToLight);

   // Linear color (before gamma correction):
   vec3 linearColor = ambient + attenuation * (diffuse + specular);
   return linearColor;
}

void main(void)
{
   // Transform normal to world coordinates:
   vec3 normal = normalize(transpose(inverse(mat3(model))) * fragmentNormal);

   // Compute vectors from the fragment (in world coordinates) to important objects in the scene:
   vec3 surfacePosition = vec3(model * vec4(fragmentVertex, 1));

   vec3 surfaceToCamera = normalize(cameraPosition - surfacePosition);

   // Calculate the contribution of each light:
   vec3 linearColor = vec3(0);
   for (int i = 0; i < 5; i++)
   {
      linearColor += GetContributionOfLight(
         allLights[i],
         fragmentColor,
         normal,
         surfacePosition,
         surfaceToCamera);
   }

   // Gamma correction:
   vec3 gamma = vec3(1.0 / 2.2);

   // Final pixel color:
   pixelColor = vec4(pow(linearColor, gamma), 1);
}
