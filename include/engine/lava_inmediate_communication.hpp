/**
 * @file lava_inmediate_communication.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Inmediate Communication's header file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_INMEDIATE_COMMUNICATION_H__
#define __LAVA_INMEDIATE_COMMUNICATION_H__ 1

#include "lava_types.hpp"

class LavaInmediateCommunication {

public:
	LavaInmediateCommunication(class LavaDevice& use_device, class LavaSurface& use_surface);
	~LavaInmediateCommunication();

	VkFence get_inmediate_fence() { return immediate_fence; }

	VkCommandBuffer get_immediate_command_buffer() { return immediate_command_buffer; }

private:
	VkFence immediate_fence;
	VkCommandBuffer immediate_command_buffer;
	VkCommandPool immediate_command_pool;

	//Require Device for creation and destruction
	class LavaDevice* device_;
};

#endif