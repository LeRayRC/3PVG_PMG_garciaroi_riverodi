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
#include <map>
#include <deque>
#include <functional>
#include <assert.h>
#include <span>
#include "vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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


#endif // ! __LAVA_CUSTOM_TYPES_
