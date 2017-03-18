#version 410 core

attribute vec4 vertex;
attribute vec4 texCoord;

uniform mat4 matrix;

varying vec4 textureCoordinate;

void main(void)
{
   gl_Position = matrix * vertex;
   textureCoordinate = texCoord;
};
