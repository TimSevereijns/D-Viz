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
   // Pass the vertex position and vertex along without modification:
   vertexPosition = vec3(instanceMatrix * vec4(vertex, 1));
   vertexColor = color;

   // Transform normal to world coordinates:
   mat4 transposedInverseInstance = transpose(inverse(instanceMatrix));
   vec3 rawVertexNormal = vec3(transposedInverseInstance * vec4(normal, 1));
   vertexNormal = normalize(rawVertexNormal);

   // Apply project, view, model, and transformation matrices:
   gl_Position = projectionMatrix * viewMatrix * instanceMatrix * vec4(vertex, 1);
}
