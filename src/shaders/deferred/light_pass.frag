#version 450

#extension GL_GOOGLE_include_directive : require
#include "..\input_structures.glsl"



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

//layout(set = 2, binding = 1) uniform LightViewProj{
//  mat4 viewproj[6];
//}light_viewproj;
//layout(set = 2, binding = 2) uniform sampler2D  shadowMap;
//layout(set = 2, binding = 3) uniform samplerCube depthMap;
//layout(set = 2, binding = 4) uniform sampler2DArray directionalShadowMaps;


layout (location = 0) in vec2 inUV;

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



void main() {

    vec3 albedo = texture(baseColorTex, inUV).rgb;
    vec3 normal = normalize(texture(normalTex, inUV).rgb);
    vec3 position = texture(positionTex, inUV).rgb;

    vec3 lighting = vec3(0.0,0.0,0.0);

    if(light.enabled == 1) {
        switch(light.type) {
            case 0: // Directional
                lighting += DirectionalLight(position, normal);
                break;
            case 1: // Point
                lighting += PointLight(position, normal);
                break;
            case 2: // Spot
                lighting += SpotLight(position, normal);
                break;
            default:
                break;
        }
    }
    
    outFragColor = vec4(albedo*lighting, 1.0);

		//if((globalData.gbuffer_render_selected & (1<<2)) == (1<<2)){
		//	outFragColor = texture(baseColorTex,inUV);
		//}else if((globalData.gbuffer_render_selected & (1 << 1)) == (1 << 1)){
		//	outFragColor = texture(normalTex,inUV);
		//}
}