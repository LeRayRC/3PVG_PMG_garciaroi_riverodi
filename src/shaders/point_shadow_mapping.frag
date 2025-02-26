#version 450

#extension GL_GOOGLE_include_directive : require

layout (location = 0) out vec4 outFragColor;

layout(set = 2, binding = 0) uniform LightProperties{
    vec3 pos;
    int enabled;

    vec3 dir;
    int type;
    
    vec3 diff_color;
    float quad_att;
    
    vec3 spec_color;
    float linear_att;
    
    vec3 spot_dir;
    float constant_att;
    
    float shininess;
    float strength;
    float cutoff;
    float outer_cutoff;
} light;

layout (location = 0) in vec4 FragPos;

void main() 
{
  outFragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);

    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - light.pos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / 25.0; //NEED TO BE A PROPERTTY
    
    // write this as modified depth
    gl_FragDepth = 1.0 - clamp(lightDistance, 0.0, 1.0);
}