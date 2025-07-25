#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "input_structures.glsl"

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;

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
	gl_Position = globalData.viewproj * PushConstants.render_matrix *vec4(v.position, 1.0);
	outColor = v.color.xyz;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	outNormal = normalize(PushConstants.render_matrix * vec4(v.normal,0.0)).xyz;
}
