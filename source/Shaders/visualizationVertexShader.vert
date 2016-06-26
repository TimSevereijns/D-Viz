#version 410 core

layout (location = 0) in vec3 color;
layout (location = 1) in mat4 instanceMatrix;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 vertex;
//in vec3 normal;

out vec3 fragmentVertex;
out vec3 fragmentColor;
//out vec3 fragmentNormal;

void main(void)
{
   // Pass color and normal values along without modification.
   fragmentVertex = vertex;
   //fragmentNormal = normal;
   fragmentColor = color;

   // @debug Verify that the correct color is passed in.
   if (color != vec3(0.5, 1, 0.5))
   {
      //fragmentColor = vec3(1, 0, 0);
   }

   // @debug Verify that the correct matrix is passed in.
   if (instanceMatrix[0] != vec4(10, 0, 0, 0))
   {
      //fragmentColor = vec3(0, 0, 1);
   }
   else if(instanceMatrix[1] != vec4(0, 10, 0, 0))
   {
      //fragmentColor = vec3(0, 0, 1);
   }
   else if(instanceMatrix[2] != vec4(0, 0, 10, 0))
   {
      //fragmentColor = vec3(0, 0, 1);
   }
   else if(instanceMatrix[3] != vec4(0, 0, 0, 1))
   {
      //fragmentColor = vec3(0, 0, 1);
   }

   // Apply project, view, model, and transformation matrices:
   gl_Position = projectionMatrix * viewMatrix * instanceMatrix * vec4(vertex, 1);
}
