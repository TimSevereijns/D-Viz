#version 130

uniform mat4 mvpMatrix;

in vec4 vertex;
in vec4 color;
in vec3 normal;

out vec4 fragmentVertex;
out vec4 fragmentColor;
out vec3 fragmentNormal;

void main(void)
{
   // Pass color and normal values along without modification.
   fragmentVertex = vertex;
   fragmentColor = color;
   fragmentNormal = /*mat3(mvpMatrix) * */normal;

   // Apply model, view, and perspective transformations to all verices.
   gl_Position = mvpMatrix * vertex;
}

