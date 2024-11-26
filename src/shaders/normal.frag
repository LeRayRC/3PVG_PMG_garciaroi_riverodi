#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{
		outFragColor = vec4(inNormal,1.0);
}