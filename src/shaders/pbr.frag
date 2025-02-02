#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

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

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;
layout (location = 4) in vec3 TangentLightPos;
layout (location = 5) in vec3 TangentViewPos;
layout (location = 6) in vec3 TangentFragPos;

//output write
layout (location = 0) out vec4 outFragColor;


vec3 final_color = vec3(0.0, 0.0, 0.0);


vec3 DirectionalLight(){
  vec3 normal_norm = normalize(inNormal);
  float directionalIncidence = max(dot(normal_norm, light.dir), 0.0);
  //Specular
  vec3 viewDirection = normalize(globalData.cameraPos - inPos);
  vec3 reflectDirection = reflect(-light.dir, normal_norm);

  float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

  vec3 diffuse = directionalIncidence * light.diff_color;
  vec3 specular = light.strength * specularValue * light.spec_color;
  return diffuse + specular;
}

vec3 PointLight() {
    vec3 lightDir = normalize(light.pos - inPos);
    float directionalIncidence = max(dot(inNormal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(globalData.cameraPos - inPos);
    vec3 reflectDirection = reflect(-lightDir, inNormal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - inPos);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);
    return diffuse * attenuation + specular * attenuation;
}

vec3 SpotLight() {
    vec3 lightDir = normalize(light.pos - inPos);
    float theta = dot(lightDir, normalize(-light.dir));
    vec3 result = vec3(0.0, 0.0, 0.0);

    float directionalIncidence = max(dot(inNormal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(globalData.cameraPos - inPos);
    vec3 reflectDirection = reflect(-lightDir, inNormal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - inPos);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);

    // Intensity
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    result = (diffuse * intensity * attenuation) + (specular * intensity * attenuation);
    return result;
}

void main() 
{
  //vec4 texColor = texture(baseColorTex,inUV);
  if(light.enabled == 1){
    switch(light.type){
      case 0: {
        outFragColor = vec4(DirectionalLight(),1.0); 
        break;
       }
       case 1: {
        outFragColor = vec4(PointLight(),1.0); 
        break;
       }
       default:{
        outFragColor = vec4(SpotLight(),1.0); 
        break;
       }
    }
  }
  //outFragColor *= texture(baseColorTex,inUV); 
}