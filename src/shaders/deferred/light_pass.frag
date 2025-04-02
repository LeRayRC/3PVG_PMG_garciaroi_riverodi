#version 450

#extension GL_GOOGLE_include_directive : require
#include "..\input_structures.glsl"



layout(set = 1, binding = 0) uniform sampler2D baseColorTex;
layout(set = 1, binding = 1) uniform sampler2D normalTex;
layout(set = 1, binding = 2) uniform sampler2D metallicRogTex;
layout(set = 1, binding = 3) uniform sampler2D opacityTex;
layout(set = 1, binding = 4) uniform LavaPBRMaterialProperties {
	float metallic_factor_;
	float roughness_factor_; 
	float specular_factor_;
	float opacity_mask_;
	float use_normal_;
}properties;


layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outPos;


void main() {
    outPos = texture(baseColorTex,inUV);

}