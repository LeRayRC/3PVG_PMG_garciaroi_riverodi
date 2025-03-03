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
layout(set = 2, binding = 1) uniform  LightViewProj{   
	mat4 viewproj[6];
} light_viewproj;
layout(set = 2, binding = 2) uniform sampler2D  shadowMap;
layout(set = 2, binding = 3) uniform samplerCube depthMap;
layout(set = 2, binding = 4) uniform sampler2DArray directionalShadowMaps;


//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inPos;
layout (location = 4) in vec3 TangentLightPos;
layout (location = 5) in vec3 TangentViewPos;
layout (location = 6) in vec3 TangentFragPos;
layout (location = 7) in vec4 fragPosLightSpace;

//output write
layout (location = 0) out vec4 outFragColor;


vec3 final_color = vec3(0.0, 0.0, 0.0);


vec3 DirectionalLight(){
  vec3 normal_norm = normalize(inNormal);
  float directionalIncidence = max(dot(normal_norm, light.dir), 0.0);
  //Specular
  vec3 viewDirection = normalize(globalData.cameraPos - inPos.xyz);
  vec3 reflectDirection = reflect(-light.dir, normal_norm);

  float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

  vec3 diffuse = directionalIncidence * light.diff_color;
  vec3 specular = light.strength * specularValue * light.spec_color;
  return diffuse + specular;
}

vec3 PointLight() {
    vec3 lightDir = normalize(light.pos - inPos.xyz);
    float directionalIncidence = max(dot(inNormal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(globalData.cameraPos - inPos.xyz);
    vec3 reflectDirection = reflect(-lightDir, inNormal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - inPos.xyz);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);
    return diffuse * attenuation + specular * attenuation;
}

vec3 SpotLight() {
    vec3 lightDir = normalize(light.pos - inPos.xyz);
    float theta = dot(lightDir, normalize(light.dir));
    vec3 result = vec3(0.0, 0.0, 0.0);

    float directionalIncidence = max(dot(inNormal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(globalData.cameraPos - inPos.xyz);
    vec3 reflectDirection = reflect(-lightDir, inNormal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - inPos.xyz);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);

    // Intensity
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    result = (diffuse * intensity * attenuation) + (specular * intensity * attenuation);
    return result;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{ 
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  float currentDepth = projCoords.z;
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float shadow = currentDepth + 0.005 < closestDepth  ? 1.0 : 0.0;
  return shadow;
}

float PointShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.pos;
    
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthMap, normalize(fragToLight)).r;
    
    closestDepth = 1.0 - closestDepth;
    
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= 25.0; //Need to be Recive: Far Plane
    
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth-bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}  

float DirectionalShadowCalculation(vec3 fragPos){

  for(int i = 0; i < 3; i++){
    vec4 fragPosLight = light_viewproj.viewproj[i] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    float currentDepth = projCoords.z;
    projCoords = projCoords * 0.5 + 0.5;  

    if(projCoords.x > 0.0 && projCoords.x < 1.0 
      && projCoords.y > 0.0 && projCoords.y < 1.0){
          return currentDepth + 0.005 < texture(directionalShadowMaps, vec3(projCoords.xy, i)).r  ? 1.0 : 0.0;
    }
  }

  return 0.0;
}


void main() 
{

 


  if(light.enabled == 1){
    
    

    switch(light.type){
      case 0: {
        float shadow_fr = DirectionalShadowCalculation(inPos.xyz);
        outFragColor = vec4(DirectionalLight() * (1.0 - shadow_fr),1.0); 
        break;
       }
       case 1: {
        float shadow_fr = PointShadowCalculation(inPos.xyz);
        outFragColor = vec4(PointLight() * (1.0 - shadow_fr),1.0); 
        break;
       }
       case 2: {
       float shadow_fr = ShadowCalculation(fragPosLightSpace); 
        vec3 lightColor = SpotLight();
        outFragColor = vec4(lightColor * (1.0 - shadow_fr), 1.0);
        break;
       }
       default:{
        break;
       }
    }
  }
  outFragColor *= texture(baseColorTex,inUV);
  outFragColor.xyz += globalData.ambientColor;
}