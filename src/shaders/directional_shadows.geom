#version 450
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(set = 2, binding = 1) uniform  LightViewProj{   
	mat4 viewproj[3];
} light_viewproj;

layout (location = 0) out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 3; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = light_viewproj.viewproj[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}