#version 410 core

//attribute vec4 vertex;
//attribute vec4 texCoord;

//uniform mat4 matrix;

//varying vec4 textureCoordinate;

//void main(void)
//{
//   gl_Position = matrix * vertex;
//   textureCoordinate = texCoord;
//};

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(position, 1.0f);
    TexCoords = texCoords;
}
