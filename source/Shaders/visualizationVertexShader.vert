#version 410 core

layout (location = 0) in vec3 color;
layout (location = 1) in mat4 instanceMatrix;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 vertex;
in vec3 normal;

//out vec3 fragmentVertex;
out vec3 vertexColor;
out vec3 vertexNormal;

void main(void)
{
   // Pass color and normal values along without modification.
   //fragmentVertex = vertex;
   vertexNormal = normal;
   vertexColor = color;

   vertexNormal = normalize(vec3(instanceMatrix * vec4(normal, 1)));

   // Apply project, view, model, and transformation matrices:
   gl_Position = projectionMatrix * viewMatrix * instanceMatrix * vec4(vertex, 1);
}
