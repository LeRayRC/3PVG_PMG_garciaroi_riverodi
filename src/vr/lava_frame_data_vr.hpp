#ifndef __LAVA_FRAME_DATA_VR_H__
#define __LAVA_FRAME_DATA_VR_H__ 1

struct FrameDataVR {
  VkFence render_fence;
  VkCommandBuffer main_command_buffer;
  VkSemaphore swap_chain_semaphore; //GPU <-> GPU
  VkSemaphore render_semaphore; //GPU <-> GPU
  VkCommandPool command_pool;
};

#endif
