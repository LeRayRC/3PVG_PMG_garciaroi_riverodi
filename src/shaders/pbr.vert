#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "input_structures.glsl"

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec4 outPos;
layout (location = 4) out vec3 tangentLightPos;
layout (location = 5) out vec3 tangentViewPos;
layout (location = 6) out vec3 tangentFragPos;
layout (location = 7) out vec4 fragPosLightSpace;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;

	vec3 tangent;
	float padding1;
	vec3 bitangent;
	float padding2;
}; 

layout(set = 1, binding = 4) uniform LavaPBRMaterialProperties {
	float metallic_factor_;
	float roughness_factor_; 
	float specular_factor_;
	float opacity_mask_;
	float use_normal_;
}properties;

layout(set = 2, binding = 1) uniform  LightViewProj{   
	mat4 viewproj[];
} light_viewproj;


layout(set = 2, binding = 2) uniform sampler2D  shadowMap;


layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 render_matrix;
	VertexBuffer vertexBuffer;
} PushConstants;



void main() 
{	
	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	vec4 pos = PushConstants.render_matrix *vec4(v.position, 1.0);
	outPos = pos;

	gl_Position = globalData.viewproj * pos;
	outColor = v.color.xyz;

	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	outNormal = normalize(PushConstants.render_matrix * vec4(v.normal,0.0)).xyz;

	//Light pos space 
	fragPosLightSpace = light_viewproj.viewproj[0] * pos;

	//Normal Mapping Calculations
    vec3 T = normalize(vec3(PushConstants.render_matrix * vec4(v.tangent,   0.0)));
    vec3 B = normalize(vec3(PushConstants.render_matrix * vec4(v.bitangent, 0.0)));
    vec3 N = normalize(vec3(PushConstants.render_matrix * vec4(v.normal,    0.0)));
    mat3 TBN = transpose(mat3(T, B, N));
    //vs_out.TangentLightPos = TBN * lightPos; //TO DO: LIGHTS
    //tangentViewPos  = TBN * viewPos; //TO DO: LIGHTS
    tangentFragPos  = TBN * pos.xyz;

	
}
