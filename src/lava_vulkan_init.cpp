#include "lava_vulkan_inits.hpp"

VkSemaphoreSubmitInfo vkinit::SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask,
  VkSemaphore semaphore) {

  VkSemaphoreSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;
  submitInfo.semaphore = semaphore;
  submitInfo.stageMask = stageMask;
  submitInfo.deviceIndex = 0;
  submitInfo.value = 1;

  return submitInfo;
}

VkCommandBufferSubmitInfo vkinit::CommandBufferSubmitInfo(VkCommandBuffer cmd) {

  VkCommandBufferSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  info.pNext = nullptr;
  info.commandBuffer = cmd;
  info.deviceMask = 0;

  return info;
}

VkSubmitInfo2 vkinit::SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
  VkSemaphoreSubmitInfo* waitSemaphoreInfo) {

  VkSubmitInfo2 info = {};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  info.pNext = nullptr;

  info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
  info.pWaitSemaphoreInfos = waitSemaphoreInfo;

  info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
  info.pSignalSemaphoreInfos = signalSemaphoreInfo;

  info.commandBufferInfoCount = 1;
  info.pCommandBufferInfos = cmd;

  return info;
}


VkImageCreateInfo vkinit::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {

  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;

  info.imageType = VK_IMAGE_TYPE_2D;

  info.format = format;
  info.extent = extent;

  info.mipLevels = 1;
  info.arrayLayers = 1;

  //Util para MSAA (Antialiasing)
  info.samples = VK_SAMPLE_COUNT_1_BIT;

  //Permite a la GPU realizar operaciones de intercambio sobre los datos 
  // de la imagen, lo cual es mas optimo
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usageFlags;

  return info;
}

VkImageViewCreateInfo vkinit::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags){
  VkImageViewCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;

  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  info.image = image;
  info.format = format;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = 1;
  info.subresourceRange.aspectMask = aspectFlags;

  return info;
}

VkCommandBufferBeginInfo vkinit::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags) {

  VkCommandBufferBeginInfo command_buffer_begin_info{};
  command_buffer_begin_info.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_buffer_begin_info.pNext = nullptr;
  command_buffer_begin_info.pInheritanceInfo = nullptr;
  command_buffer_begin_info.flags = flags;

  return command_buffer_begin_info;
}

VkRenderingAttachmentInfo vkinit::AttachmentInfo(
  VkImageView view, VkClearValue* clear, VkImageLayout layout) {
  VkRenderingAttachmentInfo color_attachment{};
  color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  color_attachment.pNext = nullptr;

  color_attachment.imageView = view;
  color_attachment.imageLayout = layout;
  color_attachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  if (clear) {
    color_attachment.clearValue = *clear;
  }

  return color_attachment;
}




VkRenderingInfo vkinit::RenderingInfo(VkExtent2D render_extent, VkRenderingAttachmentInfo* color_attachment,
  VkRenderingAttachmentInfo* depth_attachment)
{
  VkRenderingInfo render_info{};
  render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  render_info.pNext = nullptr;

  render_info.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, render_extent };
  render_info.layerCount = 1;
  render_info.colorAttachmentCount = 1;
  render_info.pColorAttachments = color_attachment;
  render_info.pDepthAttachment = depth_attachment;
  render_info.pStencilAttachment = nullptr;

  return render_info;
}

