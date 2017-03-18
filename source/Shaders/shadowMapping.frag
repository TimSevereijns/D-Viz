#version 410 core

out vec4 pixelColor;

float near = 1.0;
float far  = 1000.0;

float LinearizeDepth(float depth)
{
   float z = depth * 2.0 - 1.0; // Back to NDC
   return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{
   //float depth = LinearizeDepth(gl_FragCoord.z);
   float depth = gl_FragCoord.z;
   pixelColor = vec4(depth, depth, depth, 1.0);
}
