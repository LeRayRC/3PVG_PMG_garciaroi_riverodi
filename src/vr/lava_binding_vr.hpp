#ifndef __LAVA_BINDING_VR_H__
#define __LAVA_BINDING_VR_H__ 1

#include <openxr/openxr_platform.h>

class LavaBindingVR
{
public:
	LavaBindingVR(class LavaInstanceVR& instance);
	~LavaBindingVR();

  PFN_xrGetVulkanGraphicsRequirementsKHR get_graphics_requirements_binding() {
    return xrGetVulkanGraphicsRequirementsKHR;
  }

  PFN_xrGetVulkanInstanceExtensionsKHR get_vulkan_instance_extensions_binding() {
    return xrGetVulkanInstanceExtensionsKHR;
  }

  PFN_xrGetVulkanDeviceExtensionsKHR get_vulkan_device_extensions_binding() {
    return xrGetVulkanDeviceExtensionsKHR;
  }

  PFN_xrGetVulkanGraphicsDeviceKHR get_vulkan_graphics_device_binding() {
    return xrGetVulkanGraphicsDeviceKHR;
  }

private:
  PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = nullptr;
  PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = nullptr;
  PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = nullptr;
  PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = nullptr;
};


#endif // !__LAVA_BINDING_VR_H__
