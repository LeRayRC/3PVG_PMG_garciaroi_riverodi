#ifndef __CUSTOM_VULKAN_INITS_H
#define __CUSTOM_VULKAN_INITS_H 1

#include "lava_types.hpp"

namespace vkinit {

  VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

  VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);

  VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
    VkSemaphoreSubmitInfo* waitSemaphoreInfo);

  VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
  VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
  
}

#endif // !__CUSTOM_VULKAN_INITSH
