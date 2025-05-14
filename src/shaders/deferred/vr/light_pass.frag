#version 450

#extension GL_GOOGLE_include_directive : require
#include "..\..\input_structures.glsl"



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
layout(set = 1, binding = 5) uniform sampler2D positionTex;


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

layout(set = 2, binding = 1) uniform LightViewProj{
  mat4 viewproj[6];
}light_viewproj;
layout(set = 2, binding = 2) uniform sampler2D  spotShadowMap;
layout(set = 2, binding = 3) uniform samplerCube pointShadowMapCube;
layout(set = 2, binding = 4) uniform sampler2DArray directionalShadowMaps;


layout (location = 0) in vec2 inUV;
layout (location = 1) in mat4 cameraView;

layout (location = 0) out vec4 outFragColor;


vec3 DirectionalLight(vec3 position, vec3 normal){
  float directionalIncidence = max(dot(normal, light.dir), 0.0);
  //Specular
  vec3 viewDirection = normalize(globalData.cameraPos - position.xyz);
  vec3 reflectDirection = reflect(-light.dir, normal);

  float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

  vec3 diffuse = directionalIncidence * light.diff_color;
  vec3 specular = light.strength * specularValue * light.spec_color;
  return diffuse + specular;
}

vec3 PointLight(vec3 position, vec3 normal) {
    vec3 lightDir = normalize(light.pos - position);
    float directionalIncidence = max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(globalData.cameraPos - position);
    vec3 reflectDirection = reflect(-lightDir, normal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - position);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);
    return diffuse * attenuation + specular * attenuation;
}

vec3 SpotLight(vec3 position, vec3 normal) {
    vec3 lightDir = normalize(light.pos - position);
    float theta = dot(lightDir, normalize(light.dir));
    vec3 result = vec3(0.0, 0.0, 0.0);

    float directionalIncidence = max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 viewDirection = normalize(globalData.cameraPos - position);
    vec3 reflectDirection = reflect(-lightDir, normal);

    float specularValue = pow(max(dot(viewDirection, reflectDirection), 0.0), light.shininess);

    vec3 diffuse = directionalIncidence * light.diff_color;
    vec3 specular = light.strength * specularValue * light.spec_color;
    // Attenuation
    float distance = length(light.pos - position);
    float attenuation = 1.0 / (light.constant_att + light.linear_att * distance + light.quad_att * distance * distance);

    // Intensity
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    result = (diffuse * intensity * attenuation) + (specular * intensity * attenuation);
    return result;
}

float SpotShadowCalculation(vec3 position)
{ 
  vec4 fragPosLightSpace = light_viewproj.viewproj[0] * vec4(position, 1.0);

  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  float currentDepth = projCoords.z;
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(spotShadowMap, projCoords.xy).r;
  float shadow = currentDepth + 0.005 < closestDepth  ? 1.0 : 0.0;
  return shadow;
}

float DirectionalShadowCalculation(vec3 fragPos, vec3 normal){

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



void main() {

    vec3 albedo = texture(baseColorTex, inUV).rgb;
    vec3 normal = normalize(texture(normalTex, inUV).rgb);
    vec3 position = texture(positionTex, inUV).rgb;

    vec3 lighting = vec3(0.0,0.0,0.0);
    float shadow = 0.0;
    vec3 lightColor = vec3(0.0);
    if(light.enabled == 1) {
        switch(light.type) {
            case 0: // Directional
                shadow = DirectionalShadowCalculation(position, normal);
                lightColor = DirectionalLight(position, normal);
                lighting += lightColor * (1.0 - shadow);
                break;
            case 1: // Point
                lighting += PointLight(position, normal);
                break;
            case 2: // Spot
                shadow = SpotShadowCalculation(position);
                lightColor = SpotLight(position, normal);
                lighting += lightColor * (1.0 - shadow);
                break;
            default:
                break;
        }
    }
    
    outFragColor = vec4(albedo*lighting, 1.0);
    //outFragColor = vec4(1.0);

		//if((globalData.gbuffer_render_selected & (1<<2)) == (1<<2)){
		//	outFragColor = texture(baseColorTex,inUV);
		//}else if((globalData.gbuffer_render_selected & (1 << 1)) == (1 << 1)){
		//	outFragColor = texture(normalTex,inUV);
		//}
}