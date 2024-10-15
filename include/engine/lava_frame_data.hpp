/**
 * @file lava_frame_data.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Frame Data's header file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_FRAME_DATA_H__
#define __LAVA_FRAME_DATA_H__ 1

#include "lava_types.hpp"

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;
	VkSemaphore swap_chain_semaphore; //GPU <-> GPU
	VkSemaphore render_semaphore; //GPU <-> GPU
	VkFence render_fence; // CPU <-> GPU
	//	DeletionQueue deletion_queue;
};

 //Numero de buffers en paralelo (double-buffering)
constexpr unsigned int FRAME_OVERLAP = 2;

class LavaFrameData {

public:
	LavaFrameData(class LavaDevice& use_device, class LavaSurface& use_surface);
	~LavaFrameData();

	FrameData& getCurrentFrame() { return frames_[frame_number_ % FRAME_OVERLAP]; };

	void increaseFrameNumber() { frame_number_++; };
private:
	FrameData frames_[FRAME_OVERLAP];
	uint64_t  frame_number_;

	//Require Device for creation and destruction
	class LavaDevice* device_;
};

#endif //__LAVA_FRAME_DATA_H__