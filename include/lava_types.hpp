#ifndef __LAVA_TYPES_
#define __LAVA_TYPES_ 1

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <iostream>
#include <string>
#include <set>
#include <limits>
#include <algorithm>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <vector>
#include <queue>
#include <array>
#include <map>
#include <deque>
#include <functional>
#include <assert.h>
#include <span>
#include <format>
#include "vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include "engine/lava_buffer.hpp"

#pragma region VulkanGraphic

struct ComputePushConstants {
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};

struct ComputeEffect {
	const char* name;

	VkPipeline pipeline;
	VkPipelineLayout layout;

	bool use_push_constants;

	ComputePushConstants data;
};


struct AllocatedImage {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;
};

struct Vertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

struct VertexWithTangents {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
	glm::vec3 tangent_;
	float padding1;
	glm::vec3 bitangent_;
	float padding2;
};


struct GPUMeshBuffers {
	std::unique_ptr<LavaBuffer> index_buffer;
	std::unique_ptr<LavaBuffer> vertex_buffer;
	VkDeviceAddress vertex_buffer_address;
};


// push constants for our mesh object draws
struct GPUDrawPushConstants {
	glm::mat4 world_matrix;
	VkDeviceAddress vertex_buffer;
};

#pragma endregion

#pragma region Input

//Key Actions
#define KEY_PRESS 0x0001
#define KEY_RELEASE 0x0002
#define KEY_REPEAT 0x0004
#define SUSTAIN_TIL_RELEASE 0x0008

//Redefine Keys for easy use(can use glfw key defines for those not included)
#define KEY_SPACE 32
#define KEY_ESCAPE 256
#define KEY_ENTER 257
#define KEY_TAB 258
#define KEY_BACKSPACE 259
#define KEY_RIGHT 262
#define KEY_LEFT 263
#define KEY_DOWN 264
#define KEY_UP 265

//Redefine Mouse Buttons for easy use(can use glfw key defines for those not included)
#define 	MOUSE_BUTTON_1   0
#define 	MOUSE_BUTTON_2   1
#define 	MOUSE_BUTTON_3   2

struct KeyProperties {
	KeyProperties() {
		current_frame_properties = 0;
		past_frame_properties = 0;
		last_action_time = 0;
	}
	int32_t current_frame_properties;
	int32_t past_frame_properties;

	double last_action_time;
};

typedef struct GamepadStateWrapper {
	GamepadStateWrapper() {
		for (int i = 0; i < 6; i++) {
			state.axes[i] = 0;
			state.buttons[i] = 0;
		}
		for (int i = 6; i < 15; i++) {
			state.buttons[i] = 0;
		}
		is_active = false;
	}
	GLFWgamepadstate state;
	bool is_active;
}GamepadState;

typedef struct GamePadActionWrapper {
	GamePadActionWrapper(int game_pad_, int game_pad_button_) {
		game_pad = game_pad_;
		game_pad_button = game_pad_button_;
	}
	GamePadActionWrapper() {
		game_pad = 0;
		game_pad_button = 0;
	}
	int game_pad;
	int game_pad_button;
}GamePadAction;

#pragma endregion

struct MaterialProperties {
	std::string name;

	int pipeline_flags = 0;
	const char* vertex_shader_path;
	const char* fragment_shader_path;
};

struct MaterialPBRProperties {
	std::string name;
	int pipeline_flags = 0;
	
};

typedef enum PipelineType {
	PIPELINE_TYPE_NORMAL, 
	PIPELINE_TYPE_PBR
} PipelineType;

typedef enum PipelineFlags {
	PIPELINE_USE_ATTRIBUTES = 1,					//0001
	PIPELINE_USE_PUSHCONSTANTS = 1 << 1,	//0010
	PIPELINE_USE_DESCRIPTOR_SET = 1 << 2,  //0100
} PipelineFlags;

typedef enum PipelineBlendMode {
	PIPELINE_BLEND_DISABLE,
	PIPELINE_BLEND_ONE_ONE,
	PIPELINE_BLEND_ONE_ZERO
} PipelineBlendMode;

struct PipelineConfig {
public:
	PipelineType type;
	const char* vertex_shader_path;
	const char* fragment_shader_path;
	class LavaDevice* device;
	class LavaSwapChain* swap_chain;
	class LavaDescriptorManager* descriptor_manager;
	VkDescriptorSetLayout descriptor_set_layout;
	int pipeline_flags;
	PipelineBlendMode blend_mode;
};

typedef enum MeshType {
	MESH_FBX,
	MESH_OBJ,
	MESH_GLTF,
	MESH_CUSTOM,
} MeshType;

struct MeshProperties {
	std::string name;
	MeshType type;
	std::filesystem::path mesh_path;
	class LavaPBRMaterial* material;
	std::vector<Vertex> vertex;
	std::vector<uint32_t> index;
};

struct GeoSurface {
	uint32_t start_index;
	uint32_t count;
};

struct MeshAsset {
	MeshAsset() : name{ "NoName" }, count_surfaces{ 0 }, index_count{ 0 } {};
	std::string name;
	uint16_t count_surfaces;
	uint32_t index_count;
	//GeoSurface surfaces[5];
	GPUMeshBuffers meshBuffers;
};

struct GlobalSceneData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
	glm::vec3 ambientColor;
	glm::vec3 cameraPos;
};

struct CameraParameters {
	float fov;
};

#endif // ! __LAVA_CUSTOM_TYPES_
