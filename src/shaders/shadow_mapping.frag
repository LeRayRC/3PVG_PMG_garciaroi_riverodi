#version 450

#extension GL_GOOGLE_include_directive : require

layout (location = 0) out vec4 outFragColor;

void main() 
{
  outFragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}