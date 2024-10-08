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
#include <cstdint>
#include <fstream>
#include <vector>
#include <array>
#include <deque>
#include <functional>
#include <assert.h>
#include <span>
#include "vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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

#endif // ! __LAVA_CUSTOM_TYPES_
