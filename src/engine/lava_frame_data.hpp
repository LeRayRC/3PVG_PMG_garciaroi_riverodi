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

#include "engine/lava_descriptor_manager.hpp"
#include "lava/engine/lava_buffer.hpp"
#include "lava/engine/lava_engine.hpp"

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;
	VkSemaphore swap_chain_semaphore; //GPU <-> GPU
	VkSemaphore render_semaphore; //GPU <-> GPU
	VkFence render_fence; // CPU <-> GPU
	LavaDescriptorManager descriptor_manager;
	std::shared_ptr<class LavaMesh> last_bound_mesh;
};

 //Numero de buffers en paralelo (double-buffering)
constexpr unsigned int FRAME_OVERLAP = 2;

class LavaFrameData {

public:
	LavaFrameData(class LavaDevice& use_device, 
							  class LavaSurface& use_surface, 
								class LavaAllocator& allocator, 
								GlobalSceneData* scene_data
								);
	~LavaFrameData();



	FrameData& getCurrentFrame() { return frames_[frame_number_ % FRAME_OVERLAP]; };
	FrameData& getPreviousFrame() { 
		int frame_number = frame_number_ > 0 ? frame_number_ - 1 : frame_number_;
		return frames_[(frame_number) % FRAME_OVERLAP];
	};
	int getCurrentFrameIndex() { return frame_number_ % FRAME_OVERLAP; }
	//void initGlobalDescriptorSet(VkDescriptorSetLayout layout);
	void increaseFrameNumber() { frame_number_++; };
	uint64_t  frame_number_;
private:
	FrameData frames_[FRAME_OVERLAP];
	
	//Require Device for creation and destruction
	class LavaDevice* device_;
	class LavaAllocator* allocator_;

};

#endif //__LAVA_FRAME_DATA_H__