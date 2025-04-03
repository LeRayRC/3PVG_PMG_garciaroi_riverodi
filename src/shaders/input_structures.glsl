layout(set = 0, binding = 0) uniform  GlobalData{   
	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec3 ambientColor;
	int gbuffer_render_selected;
	vec3 cameraPos;
	int padding2;
} globalData;



