#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout(set = 1, binding = 0) uniform sampler2D diffuseText;
layout(set = 1, binding = 1) uniform sampler2D normalTex;

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