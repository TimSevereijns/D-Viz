#version 410 core

layout (location = 0) in vec3 color;
layout (location = 1) in mat4 instanceMatrix;

uniform mat4 cameraProjectionViewMatrix;
uniform mat4 shadowProjectionViewMatrix;

in vec3 vertex;
in vec3 normal;

out vec3 vertexPosition;
out vec3 vertexColor;
out vec3 vertexNormal;

out vec4 shadowCoordinate;

void main(void)
{
   vertexPosition = vec3(instanceMatrix * vec4(vertex, 1.0f));
   vertexColor = color;
   vertexNormal = normal;

   shadowCoordinate = shadowProjectionViewMatrix * instanceMatrix * vec4(vertex, 1.0f);

   gl_Position = cameraProjectionViewMatrix * instanceMatrix * vec4(vertex, 1.0f);
}
