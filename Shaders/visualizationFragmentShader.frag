#version 150

uniform mat4 model;

uniform struct Light
{
   vec3 position;
   vec3 intensity;
} light;

in vec3 fragmentVertex;
in vec3 fragmentColor;
in vec3 fragmentNormal;

out vec4 pixelColor;

void main(void)
{
   // Transform the normal to world coordinates:
   mat3 normalMatrix = transpose(inverse(mat3(model)));
   vec3 normal = normalize(normalMatrix * fragmentNormal);

   // Compute the position of the fragment in world coordinates:
   vec3 fragmentPosition = vec3(model * vec4(fragmentVertex, 1));

   // Compute the vector from the current fragment to the light source:
   vec3 surfaceToLight = light.position - fragmentPosition;

   // Compute the brightness of the fragment based on the angle of incidence of the ray of light:
   float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
   brightness = clamp(brightness, 0, 1);

   // Combine all this to get the final pixel color:
   pixelColor = vec4(brightness * light.intensity * fragmentColor, 1);
}
