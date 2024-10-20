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
#include <array>
#include <deque>
#include <functional>
#include <assert.h>
#include <span>
#include <format>
#include "vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>

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

struct AllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

struct Vertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {

	AllocatedBuffer index_buffer;
	AllocatedBuffer vertex_buffer;
	VkDeviceAddress vertex_buffer_address;
};

// push constants for our mesh object draws
struct GPUDrawPushConstants {
	glm::mat4 world_matrix;
	VkDeviceAddress vertex_buffer;
};

struct MaterialProperties {
	std::string name;

	int pipeline_flags = 0;
	const char* vertex_shader_path;
	const char* fragment_shader_path;
};

typedef enum PipelineFlags {
	PIPELINE_USE_ATTRIBUTES = 1,					//0001
	PIPELINE_USE_PUSHCONSTANTS = 1 << 1,	//0010
	PIPELINE_USE_DESCRIPTOR_SET = 1 << 2,  //0100
} PipelineFlags;

struct PipelineConfig {
public:
	const char* vertex_shader_path;
	const char* fragment_shader_path;
	class LavaDevice* device;
	class LavaSwapChain* swap_chain;
	VkDescriptorSetLayout descriptor_set_layout;
	int pipeline_flags;
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
	class LavaMaterial* material;
	std::vector<Vertex> vertex;
	std::vector<uint32_t> index;
};

struct GeoSurface {
	uint32_t start_index;
	uint32_t count;
};

struct MeshAsset {
	std::string name;
	std::vector<GeoSurface> surfaces;
	GPUMeshBuffers meshBuffers;
};



#endif // ! __LAVA_CUSTOM_TYPES_
