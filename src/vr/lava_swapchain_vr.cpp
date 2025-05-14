#include "vr/lava_swapchain_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_session_vr.hpp"
#include "vr/lava_helpers_vr.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_vulkan_helpers.hpp"
#include "lava/openxr_common/OpenXRHelper.h"

LavaSwapchainVR::LavaSwapchainVR(LavaInstanceVR& instance_vr, LavaSessionVR& session, LavaDevice& device) :
  instance_vr_{instance_vr},
  device_{ device }
{
  // Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
  uint32_t format_count = 0;
  OPENXR_CHECK_INSTANCE(xrEnumerateSwapchainFormats(session.get_session(), 0, &format_count, nullptr),
    "Failed to enumerate Swapchain Formats",
    instance_vr.get_instance());

  std::vector<int64_t> formats(format_count);
  OPENXR_CHECK_INSTANCE(xrEnumerateSwapchainFormats(session.get_session(), format_count, &format_count, formats.data()),
    "Failed to enumerate Swapchain Formats",
    instance_vr.get_instance());

  if (SelectDepthSwapchainFormat(formats) == 0) {
    std::cerr << "Failed to find depth format for Swapchain." << std::endl;
    DEBUG_BREAK;
  }

  std::vector<XrViewConfigurationView>& view_configuration_views = session.get_view_configuration_views();

  color_swapchain_infos.resize(view_configuration_views.size());
  depth_swapchain_infos.resize(view_configuration_views.size());

  for (size_t i = 0; i < view_configuration_views.size(); i++) {
    SwapchainInfo& color_swapchain_info = color_swapchain_infos[i];
    SwapchainInfo& depth_swapchain_info = depth_swapchain_infos[i];

    // Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
    // Color.
    XrSwapchainCreateInfo swapchainCI{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
    swapchainCI.createFlags = 0;
    swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.format = SelectColorSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
    swapchainCI.sampleCount = view_configuration_views[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
    swapchainCI.width = view_configuration_views[i].recommendedImageRectWidth;
    swapchainCI.height = view_configuration_views[i].recommendedImageRectHeight;
    swapchainCI.faceCount = 1;
    swapchainCI.arraySize = 1;
    swapchainCI.mipCount = 1;
    OPENXR_CHECK_INSTANCE(
      xrCreateSwapchain(session.get_session(), &swapchainCI, &color_swapchain_info.swapchain),
      "Failed to create Color Swapchain",
      instance_vr_.get_instance());
    color_swapchain_info.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

    // Depth.
    swapchainCI.createFlags = 0;
    swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    swapchainCI.format = SelectDepthSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
    swapchainCI.sampleCount = view_configuration_views[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
    swapchainCI.width = view_configuration_views[i].recommendedImageRectWidth;
    swapchainCI.height = view_configuration_views[i].recommendedImageRectHeight;
    swapchainCI.faceCount = 1;
    swapchainCI.arraySize = 1;
    swapchainCI.mipCount = 1;
    OPENXR_CHECK_INSTANCE(
      xrCreateSwapchain(session.get_session(), &swapchainCI, &depth_swapchain_info.swapchain),
      "Failed to create Depth Swapchain",
      instance_vr.get_instance());
    depth_swapchain_info.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.
  


    // Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
    uint32_t color_swapchain_image_count = 0;
    OPENXR_CHECK_INSTANCE(xrEnumerateSwapchainImages(color_swapchain_info.swapchain, 0, &color_swapchain_image_count, nullptr),
      "Failed to enumerate Color Swapchain Images.",
      instance_vr.get_instance());
    XrSwapchainImageBaseHeader* colorSwapchainImages = allocateSwapchainImageData(color_swapchain_info.swapchain, SwapchainType::COLOR, color_swapchain_image_count);
    
    OPENXR_CHECK_INSTANCE(xrEnumerateSwapchainImages(color_swapchain_info.swapchain, color_swapchain_image_count, &color_swapchain_image_count, colorSwapchainImages),
      "Failed to enumerate Color Swapchain Images.",
      instance_vr.get_instance());

    uint32_t depth_swapchain_image_count = 0;
    OPENXR_CHECK_INSTANCE(xrEnumerateSwapchainImages(depth_swapchain_info.swapchain, 0, &depth_swapchain_image_count, nullptr),
      "Failed to enumerate Depth Swapchain Images.",
      instance_vr.get_instance());

    XrSwapchainImageBaseHeader* depth_swapchain_images = allocateSwapchainImageData(depth_swapchain_info.swapchain, SwapchainType::DEPTH, depth_swapchain_image_count);
    
    OPENXR_CHECK_INSTANCE(xrEnumerateSwapchainImages(depth_swapchain_info.swapchain, depth_swapchain_image_count, &depth_swapchain_image_count, depth_swapchain_images),
      "Failed to enumerate Depth Swapchain Images.",
      instance_vr.get_instance());


    // Per image in the swapchains, ImageViewCreateInfo structure and create a color/depth image view.
    for (uint32_t j = 0; j < color_swapchain_image_count; j++) {
      VkImage image = GetSwapchainImage(color_swapchain_info.swapchain, j);
      VkImageViewCreateInfo image_view_create_info = vkinit::ImageViewCreateInfo(
        VK_IMAGE_VIEW_TYPE_2D, (VkFormat)color_swapchain_info.swapchainFormat,
        image, VK_IMAGE_ASPECT_COLOR_BIT
      );
      VkImageView image_view;
      VULKAN_CHECK(vkCreateImageView(device_.get_device(), &image_view_create_info, nullptr, &image_view), "Failed to create ImageView.");
      image_view_resources[image_view] = image_view_create_info;
      color_swapchain_info.imageViews.push_back((void*)image_view);
    }

    for (uint32_t j = 0; j < depth_swapchain_image_count; j++) {
      VkImage image = GetSwapchainImage(depth_swapchain_info.swapchain, j);
      VkImageViewCreateInfo image_view_create_info = vkinit::ImageViewCreateInfo(
        VK_IMAGE_VIEW_TYPE_2D, (VkFormat)depth_swapchain_info.swapchainFormat,
        image, VK_IMAGE_ASPECT_DEPTH_BIT
      );
      VkImageView image_view;
      VULKAN_CHECK(vkCreateImageView(device_.get_device(), &image_view_create_info, nullptr, &image_view), "Failed to create ImageView.");
      image_view_resources[image_view] = image_view_create_info;
      depth_swapchain_info.imageViews.push_back((void*)image_view);
    }
  }
}


XrSwapchainImageBaseHeader* LavaSwapchainVR::allocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count) {
  swapchain_images_map[swapchain].first = type;
  swapchain_images_map[swapchain].second.resize(count, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
  return reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain_images_map[swapchain].second.data());
}

void LavaSwapchainVR::freeSwapchainImageData(XrSwapchain swapchain) {
  swapchain_images_map[swapchain].second.clear();
  swapchain_images_map.erase(swapchain);
}

LavaSwapchainVR::~LavaSwapchainVR()
{
  vkDeviceWaitIdle(device_.get_device());
  for (size_t i = 0; i < color_swapchain_infos.size(); i++) {
    SwapchainInfo& colorSwapchainInfo = color_swapchain_infos[i];
    SwapchainInfo& depthSwapchainInfo = depth_swapchain_infos[i];

    // Destroy the color and depth image views from GraphicsAPI.
    for (void*& imageView : colorSwapchainInfo.imageViews) {
      VkImageView vkImageView = (VkImageView)imageView;
      vkDestroyImageView(device_.get_device(), vkImageView, nullptr);
      image_view_resources.erase(vkImageView);
      imageView = nullptr;
    }
    for (void*& imageView : depthSwapchainInfo.imageViews) {
      VkImageView vkImageView = (VkImageView)imageView;
      vkDestroyImageView(device_.get_device(), vkImageView, nullptr);
      image_view_resources.erase(vkImageView);
      imageView = nullptr;
    }

    // Free the Swapchain Image Data.
    freeSwapchainImageData(colorSwapchainInfo.swapchain);
    freeSwapchainImageData(depthSwapchainInfo.swapchain);

    // Destroy the swapchains.
    OPENXR_CHECK_INSTANCE(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain",instance_vr_.get_instance());
    OPENXR_CHECK_INSTANCE(xrDestroySwapchain(depthSwapchainInfo.swapchain), "Failed to destroy Depth Swapchain",instance_vr_.get_instance());
  }
}
