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


VkImageCreateInfo vkinit::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent, int layers) {

  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.pNext = nullptr;

  info.imageType = VK_IMAGE_TYPE_2D;

  info.format = format;
  info.extent = extent;

  info.mipLevels = 1;
  info.arrayLayers = layers;
  if(layers == 6)info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; //Maybe should not always be a cube when there are 6 layers

  //Util para MSAA (Antialiasing)
  info.samples = VK_SAMPLE_COUNT_1_BIT;

  //Permite a la GPU realizar operaciones de intercambio sobre los datos 
  // de la imagen, lo cual es mas optimo
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.usage = usageFlags;

  return info;
}

VkImageViewCreateInfo vkinit::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags, int layers){
  VkImageViewCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.pNext = nullptr;

  info.image = image;
  info.format = format;
  info.subresourceRange.baseMipLevel = 0;
  info.subresourceRange.levelCount = 1;
  info.subresourceRange.baseArrayLayer = 0;
  info.subresourceRange.layerCount = layers;
  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  if (layers == 6) info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  else if (layers != 1) info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
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
  VkRenderingAttachmentInfo* depth_attachment, int layers)
{
  VkRenderingInfo render_info{};
  render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  render_info.pNext = nullptr;

  render_info.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, render_extent };
  render_info.layerCount = layers;
  if (color_attachment) {
    render_info.colorAttachmentCount = 1;
    render_info.pColorAttachments = color_attachment;
  }
  else {
    render_info.colorAttachmentCount = 0;
    render_info.pColorAttachments = nullptr;
  }
  render_info.pDepthAttachment = depth_attachment;
  render_info.pStencilAttachment = nullptr;

  return render_info;
}


VkRenderingInfo vkinit::RenderingInfo(VkExtent2D render_extent, 
  VkRenderingAttachmentInfo* color_attachment,
  uint32_t color_attachment_count,
  VkRenderingAttachmentInfo* depth_attachment,
  int layers)
{
  VkRenderingInfo render_info{};
  render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  render_info.pNext = nullptr;

  render_info.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, render_extent };
  render_info.layerCount = layers;
  render_info.colorAttachmentCount = color_attachment_count;
  render_info.pColorAttachments = color_attachment;
  render_info.pDepthAttachment = depth_attachment;
  render_info.pStencilAttachment = nullptr;

  return render_info;
}

VkRenderingAttachmentInfo vkinit::DepthAttachmentInfo(
  VkImageView view, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/, VkAttachmentLoadOp loadOp, float clear_value){

  VkRenderingAttachmentInfo depth_attachment{};
  depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  depth_attachment.pNext = nullptr;

  depth_attachment.imageView = view;
  depth_attachment.imageLayout = layout;
  //depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.loadOp = loadOp;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment.clearValue.depthStencil.depth = clear_value;

  return depth_attachment;
}

