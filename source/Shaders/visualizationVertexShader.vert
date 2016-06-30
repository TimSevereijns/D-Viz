#version 410 core

layout (location = 0) in vec3 color;
layout (location = 1) in mat4 instanceMatrix;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 vertex;
in vec3 normal;

out vec3 vertexPosition;
out vec3 vertexColor;
out vec3 vertexNormal;

void main(void)
{
   vertexColor = color;

   vertexPosition = vec3(instanceMatrix * vec4(vertex, 1));

   // Transform normal to world coordinates:
   mat4 normalMatrix = transpose(inverse(instanceMatrix));
   vec3 rawVertexNormal = vec3(normalMatrix * vec4(normal, 1));
   vertexNormal = normalize(rawVertexNormal);

   // Apply projection, view, and instance matrices:
   gl_Position = projectionMatrix * viewMatrix * instanceMatrix * vec4(vertex, 1);
}
