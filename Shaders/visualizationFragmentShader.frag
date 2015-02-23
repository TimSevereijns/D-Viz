#version 150

uniform mat4 model;

uniform struct Light
{
   vec3 position;
   vec3 intensity;
} light;

in vec4 fragmentVertex;
in vec4 fragmentColor;
in vec3 fragmentNormal;

out vec4 pixelColor;

void main(void)
{
   vec3 surfaceToLightVector = light.position - vec3(fragmentVertex);

   float brightness = dot(fragmentNormal, surfaceToLightVector) /
         (length(surfaceToLightVector) * length(fragmentNormal));
   brightness = clamp(brightness, 0, 1);

   pixelColor = brightness * vec4(light.intensity, 1) * fragmentColor;
   //pixelColor = fragmentColor;
}
