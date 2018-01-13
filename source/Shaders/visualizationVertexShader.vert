#version 330 core

layout (location = 0) in vec3 color;
layout (location = 1) in mat4 instanceMatrix;

const int CASCADE_COUNT = 3;

uniform mat4 cameraProjectionViewMatrix;
uniform mat4 lightProjectionViewMatrix;

uniform mat4 lightProjectionViewMatrices[CASCADE_COUNT];

in vec3 vertex;
in vec3 normal;

out vec3 vertexPosition;
out vec3 vertexColor;
out vec3 vertexNormal;

out vec4 shadowCoordinate;

// The fragment's position in each of the shadow cascades:
out vec4 lightSpacePosition[CASCADE_COUNT];

void main(void)
{
   vertexPosition = vec3(instanceMatrix * vec4(vertex, 1.0f));
   vertexColor = color;
   vertexNormal = normal;

   shadowCoordinate = lightProjectionViewMatrix * instanceMatrix * vec4(vertex, 1.0f);

// @todo Replace the line above with this loop:
//   for (int i = 0; i < CASCADE_COUNT; i++)
//   {
//       lightSpacePosition[i] = lightProjectionViewMatrices[i] * vertexPosition;
//   }

   gl_Position = cameraProjectionViewMatrix * instanceMatrix * vec4(vertex, 1.0f);
}
