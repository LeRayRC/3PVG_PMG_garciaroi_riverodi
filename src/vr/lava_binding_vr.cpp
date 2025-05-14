#include "lava_binding_vr.hpp"
#include "lava_instance_vr.hpp"
#include "lava/openxr_common/OpenXRHelper.h"

LavaBindingVR::LavaBindingVR(LavaInstanceVR& instance)
{
  OPENXR_CHECK_INSTANCE(xrGetInstanceProcAddr(instance.get_instance(), "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirementsKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.", instance.get_instance());
  OPENXR_CHECK_INSTANCE(xrGetInstanceProcAddr(instance.get_instance(), "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanInstanceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.", instance.get_instance());
  OPENXR_CHECK_INSTANCE(xrGetInstanceProcAddr(instance.get_instance(), "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanDeviceExtensionsKHR), "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.", instance.get_instance());
  OPENXR_CHECK_INSTANCE(xrGetInstanceProcAddr(instance.get_instance(), "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDeviceKHR), "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.", instance.get_instance());
}

LavaBindingVR::~LavaBindingVR()
{
}