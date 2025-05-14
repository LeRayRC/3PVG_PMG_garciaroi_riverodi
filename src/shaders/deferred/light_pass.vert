#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "..\input_structures.glsl"



layout (location = 0) out vec2 outUV;
layout (location = 1) out mat4 cameraView;

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

	gl_Position = vec4(v.position,1.0);

	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	cameraView = mat4(1.0f);//globalData.view;
}