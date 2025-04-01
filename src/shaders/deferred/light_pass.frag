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


layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inPos;
layout (location = 4) in vec3 TangentLightPos;
layout (location = 5) in vec3 TangentViewPos;
layout (location = 6) in vec3 TangentFragPos;
//layout (location = 7) in vec4 fragPosLightSpace;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outNormal;


void main() {
    outPos = inPos;
    outNormal = vec4(normalize(inNormal) * 0.5 + 0.5,1.0);
    outAlbedo = texture(baseColorTex,inUV);
}