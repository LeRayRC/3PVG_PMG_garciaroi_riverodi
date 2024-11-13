layout(set = 0, binding = 0) uniform  GlobalData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;

} globalData;


layout(set = 1, binding = 0) uniform sampler2D diffuseText;
layout(set = 1, binding = 1) uniform sampler2D normalTex;