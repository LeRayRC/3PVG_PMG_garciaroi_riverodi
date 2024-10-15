#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) in vec3 pos;
layout (location = 1) in float uv_x;
layout (location = 2) in vec3 nor;
layout (location = 3) in float uv_y;
layout (location = 4) in vec3 colr;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex {

	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
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
	//Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = PushConstants.render_matrix *vec4(pos, 1.0f);
	outColor = colr.xyz;
	outUV.x = uv_x;
	outUV.y = uv_y;
}