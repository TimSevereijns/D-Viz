#version 330 core

out vec4 pixelColor;

void main()
{
   float depth = gl_FragCoord.z;
   pixelColor = vec4(depth, depth, depth, 1.0);
}
