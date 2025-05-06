#ifndef __LAVA_SWAPCHAIN_VR_H__
#define __LAVA_SWAPCHAIN_VR_H__ 1

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>



class LavaSwapchainVR
{
public:
	LavaSwapchainVR(class LavaInstanceVR& instance_vr,
    class LavaSessionVR& session,
    class LavaDevice& device);
	~LavaSwapchainVR();

  struct SwapchainInfo {
    XrSwapchain swapchain = XR_NULL_HANDLE;
    int64_t swapchainFormat = 0;
    std::vector<void*> imageViews;
  };

  enum class SwapchainType : uint8_t {
    COLOR,
    DEPTH
  };

  VkImage GetSwapchainImage(XrSwapchain swapchain, uint32_t index) {
    VkImage image = swapchain_images_map[swapchain].second[index].image;
    VkImageLayout layout = swapchain_images_map[swapchain].first == SwapchainType::COLOR ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    image_states_[image] = layout;
    return image;
  }

  std::vector<SwapchainInfo>& get_color_swapchain_infos() {
    return color_swapchain_infos;
  }

  std::vector<SwapchainInfo>& get_depth_swapchain_infos() {
    return depth_swapchain_infos;
  }


private:
  XrSwapchainImageBaseHeader* allocateSwapchainImageData(XrSwapchain swapchain,SwapchainType type, uint32_t count);
  void freeSwapchainImageData(XrSwapchain swapchain);
  std::unordered_map<XrSwapchain, std::pair<SwapchainType, std::vector<XrSwapchainImageVulkanKHR>>> swapchain_images_map{};
  std::unordered_map<VkImage, VkImageLayout> image_states_;
  std::unordered_map<VkImageView, VkImageViewCreateInfo> image_view_resources;


  std::vector<SwapchainInfo> color_swapchain_infos = {};
  std::vector<SwapchainInfo> depth_swapchain_infos = {};
  
  class LavaInstanceVR& instance_vr_;
  class LavaDevice& device_;
};



#endif // !__LAVA_SWAPCHAIN_VR_H__
