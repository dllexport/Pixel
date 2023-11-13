#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

layout(set = 0, binding = 0) buffer storageBuffer { float imageData[]; };

void main() 
{
  outFragcolor = vec4(1.0, 1.0, 1.0, 1.0);	
}