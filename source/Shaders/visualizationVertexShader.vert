#version 330

uniform mat4 mvpMatrix;

in vec3 vertex;
in vec3 color;
in vec3 normal;

out vec3 fragmentVertex;
out vec3 fragmentColor;
out vec3 fragmentNormal;

void main(void)
{
   // Pass color and normal values along without modification.
   fragmentVertex = vertex;
   fragmentColor = color;
   fragmentNormal = normal;

   // Apply model, view, and perspective transformations to all verices.
   gl_Position = mvpMatrix * vec4(vertex, 1);
}
