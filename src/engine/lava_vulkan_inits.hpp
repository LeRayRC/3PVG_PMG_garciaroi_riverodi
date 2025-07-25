#ifndef __CUSTOM_VULKAN_INITS_H
#define __CUSTOM_VULKAN_INITS_H 1

#include "lava/common/lava_types.hpp"

namespace vkinit {

  VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

  VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);

  VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
    VkSemaphoreSubmitInfo* waitSemaphoreInfo);

  VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, int layers = 1);
  VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, int layers = 1);
  VkImageViewCreateInfo ImageViewCreateInfo(VkImageViewType view_type, VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, int layers = 1);



  VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
  VkRenderingAttachmentInfo AttachmentInfo(
    VkImageView view, VkClearValue* clear, VkImageLayout layout);

  VkRenderingInfo RenderingInfo(VkExtent2D render_extent, VkRenderingAttachmentInfo* color_attachment,
    VkRenderingAttachmentInfo* depth_attachment, int layers = 1);

  VkRenderingAttachmentInfo DepthAttachmentInfo(
    VkImageView view, VkImageLayout layout , VkAttachmentLoadOp loadOp, float clear_value = 0.0f);

  VkRenderingInfo RenderingInfo(VkExtent2D render_extent,
    VkRenderingAttachmentInfo* color_attachment,
    uint32_t color_attachment_count,
    VkRenderingAttachmentInfo* depth_attachment,
    int layers = 1);

}

#endif // !__CUSTOM_VULKAN_INITSH
