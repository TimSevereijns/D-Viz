#version 410 core

layout (location = 0) in vec3 vertex;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 model;

void main()
{
   gl_Position = projectionMatrix * viewMatrix * model * vec4(vertex, 1.0f);
}
