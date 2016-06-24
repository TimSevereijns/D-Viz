#version 410 core

layout (location = 0) in vec3 color;
layout (location = 2) in mat4 instanceMatrix;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 vertex;
in vec3 normal;

out vec3 fragmentVertex;
out vec3 fragmentColor;
out vec3 fragmentNormal;

void main(void)
{
   // Pass color and normal values along without modification.
   fragmentVertex = vertex;
   fragmentNormal = normal;
   fragmentColor = color;

   // Apply project, view, model, and transformation matrices:
   //gl_Position = projectionMatrix * viewMatrix * vec4(vertex + vec3(instanceMatrix[3]), 1);
   gl_Position = projectionMatrix * viewMatrix * vec4(vertex + vec3(10, 0, 0), 1);
}
