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
layout (location = 4) in mat3 TBN;
layout (location = 7) in vec4 fragPosLightSpace;
layout (location = 8) in mat4 cameraView;
layout (location = 12) in vec3 inCameraPos;
layout (location = 13) in vec3 inViewDir;
//layout (location = 4) in vec3 TangentLightPos;
//layout (location = 5) in vec3 TangentViewPos;
//layout (location = 6) in vec3 TangentFragPos;

//output write
layout (location = 0) out vec4 outFragColor;


vec3 final_color = vec3(0.0, 0.0, 0.0);

vec3 useNormal;


vec3 DirectionalLight(){
  vec3 normal_norm = normalize(useNormal);
  float directionalIncidence = max(dot(normal_norm, light.dir), 0.0);
  //Specular
  vec3 viewDirection = normalize(inViewDir);
  vec3 reflectDirection = reflect(-light.dir, normal_norm);

  float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

  vec3 diffuse = directionalIncidence * light.diff_color;
  vec3 specular = light.strength * specularValue * light.spec_color;
  return diffuse + specular;
}

vec3 PointLight() {
    vec3 lightDir = normalize(light.pos - inPos.xyz);
    float directionalIncidence = max(dot(useNormal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(inViewDir);
    vec3 reflectDirection = reflect(-lightDir, useNormal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - inPos.xyz);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);
    return diffuse * attenuation + specular * attenuation;
}

vec3 SpotLight() {
    // Direction from fragment to light
    vec3 fragToLight = normalize(light.pos - inPos.xyz);
    
    // Spotlight direction (already computed in C++)
    vec3 spotDirection = normalize(-light.dir);
    
    // Calculate cosine of angle between light direction and fragment direction
    float cosTheta = dot(fragToLight, -spotDirection);
    
    // Use the PRE-COMPUTED cosine values from C++ (don't convert again!)
    float innerCutoffCos = light.cutoff;      // Already cosine from C++
    float outerCutoffCos = light.outer_cutoff; // Already cosine from C++
    
    // Calculate spotlight intensity
    float spotEffect = 0.0;
    if (cosTheta >= outerCutoffCos) {
        if (cosTheta >= innerCutoffCos) {
            spotEffect = 1.0; // Inside inner cone
        } else {
            // Smooth falloff between inner and outer
            float epsilon = innerCutoffCos - outerCutoffCos;
            spotEffect = (cosTheta - outerCutoffCos) / epsilon;
        }
    }
    
    // Early exit if outside spotlight cone
    if (spotEffect <= 0.0) {
        return vec3(0.0);
    }
    
    // Distance attenuation
    float distance = length(light.pos - inPos.xyz);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);
    
    // Diffuse lighting
    float diff = max(dot(normalize(useNormal), fragToLight), 0.0);
    vec3 diffuse = diff * light.diff_color;
    
    // Specular lighting
    vec3 viewDir = normalize(inCameraPos - inPos.xyz);
    vec3 halfwayDir = normalize(fragToLight + viewDir);
    float spec = pow(max(dot(normalize(useNormal), halfwayDir), 0.0), light.shininess);
    vec3 specular = light.strength * spec * light.spec_color;
    
    return (diffuse + specular) * spotEffect * attenuation;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{ 
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  float currentDepth = projCoords.z;
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float shadow = currentDepth + 0.00005 < closestDepth  ? 1.0 : 0.0;
  return shadow;
}

float DirectShadowCalculation(vec4 fragPosLightSpace)
{ 
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  float currentDepth = projCoords.z;
  float closestDepth = texture(directionalShadowMaps, vec3(projCoords.xy,0.0)).r;
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

    vec4 fragPosViewSpace = cameraView * vec4(fragPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);
    
    int layer = -1;
    float planeStep = 50.0 / 3.0;
    for (int i = 0; i < 3; ++i)
    {
        float curPlaneStep = (float(i + 1)) * planeStep;
        if (depthValue < curPlaneStep)
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = 2;
    }
        
    vec4 fragmPosLightSpace = light_viewproj.viewproj[layer] * vec4(fragPos, 1.0);
        
    // get depth of current fragment from light's perspective
    float currentDepth = fragmPosLightSpace.z;
    if (currentDepth  > 1.0)
    {
        return 0.0;
    }

    // perform perspective divide
    vec3 projCoords = fragmPosLightSpace.xyz / fragmPosLightSpace.w;

    // transform to [0,1] range
    projCoords = (projCoords * 0.5) + 0.5;


    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(useNormal);
    float bias = max(0.05 * (1.0 - dot(normal, light.dir)), 0.005);
    if (layer == 2)
    {
        bias *= 1 / (50.0 * 0.5);
    }
    else
    {
        float planeDistance = planeStep * (float(layer+1));
        bias *= 1 / (0.5 * planeDistance);
    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(directionalShadowMaps, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(
                        directionalShadowMaps,
                        vec3(projCoords.xy + (vec2(x, y) * texelSize),
                        layer)
                        ).r; 
            shadow += ((currentDepth - bias) > pcfDepth) ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }
        	
    return shadow;
}


void main() 
{
  vec4 albedo = texture(baseColorTex,inUV);
  if(albedo.a < properties.opacity_mask_) discard;

  vec3 normalMap = texture(normalTex, inUV).rgb;
  normalMap = normalMap * 2.0 - 1.0;   
  normalMap = normalize(TBN * normalMap); 
  useNormal = (properties.use_normal_ == 0.0) ? inNormal : normalMap;
  useNormal = normalize(useNormal);

  if(light.enabled == 1){
    switch(light.type){
      case 0: {
        float shadow_fr = 1.0 - DirectionalShadowCalculation(inPos.xyz);
        vec3 lightColor = DirectionalLight();
        outFragColor = vec4(lightColor * shadow_fr,1.0); 
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

  outFragColor.xyz += globalData.ambientColor;
  outFragColor *= albedo;
}