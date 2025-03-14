#version 450
layout (triangles, invocations = 3) in;
layout (triangle_strip, max_vertices=3) out;

layout(set = 2, binding = 1) uniform  LightViewProj{   
	mat4 viewproj[3];
} light_viewproj;

//layout (location = 0) out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{

    for (int i = 0; i < 3; ++i)
    {
        gl_Position = 
            light_viewproj.viewproj[gl_InvocationID] * gl_in[i].gl_Position;;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}