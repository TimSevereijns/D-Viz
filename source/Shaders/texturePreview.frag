#version 330 core

uniform sampler2D texture;
varying vec4 textureCoordinate;

void main(void)
{
   gl_FragColor = texture2D(texture, textureCoordinate.st);
};
