#version 130

uniform mat4 mvpMatrix;

in vec3 vertex;
in vec3 color;
in vec3 normal;

out vec3 vertexPosition;
out vec3 vertexColor;
out vec3 vertexNormal;

void main(void)
{
   // Pass color and normal values along without modification.
   vertexPosition = vertex;
   vertexColor = color;
   vertexNormal = normal;

   // Apply model, view, and perspective transformations to all verices.
   gl_Position = mvpMatrix * vec4(vertex, 1);
}
