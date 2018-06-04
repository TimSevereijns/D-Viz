#version 330 core

layout (location = 0) in vec3 color;
layout (location = 1) in mat4 instanceMatrix;

uniform mat4 cameraProjectionViewMatrix;
uniform mat4 lightProjectionViewMatrix;

const int CASCADE_COUNT = 4;
uniform mat4 lightProjectionViewMatrices[CASCADE_COUNT];

in vec3 vertex;
in vec3 normal;

out vec3 vertexPosition;
out vec3 vertexColor;
out vec3 vertexNormal;

out vec4 shadowCoordinates[CASCADE_COUNT];

out float clipSpaceZ;

void main(void)
{
   vertexPosition = vec3(instanceMatrix * vec4(vertex, 1.0f));
   vertexColor = color;
   vertexNormal = normal;

   for (int i = 0; i < CASCADE_COUNT; ++i)
   {
      shadowCoordinates[i] = lightProjectionViewMatrices[i] * instanceMatrix * vec4(vertex, 1.0f);
   }

   gl_Position = cameraProjectionViewMatrix * instanceMatrix * vec4(vertex, 1.0f);
   clipSpaceZ = gl_Position.z;
}
