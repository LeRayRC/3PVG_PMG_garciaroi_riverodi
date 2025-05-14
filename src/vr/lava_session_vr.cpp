#include "lava_session_vr.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_device.hpp"
#include "vr/lava_instance_vr.hpp"
#include "lava/openxr_common/OpenXRHelper.h"
#include <openxr/openxr_platform.h>

LavaSessionVR::LavaSessionVR(LavaInstanceVR& instance_vr, 
  LavaInstance& instance, LavaDevice& device, XrViewConfigurationType view_configuration_type) : instance_vr_{ instance_vr }
{
  XrGraphicsBindingVulkanKHR graphics_binding{};

  graphics_binding = { XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
  graphics_binding.instance = instance.get_instance();
  graphics_binding.physicalDevice = device.get_physical_device();
  graphics_binding.device = device.get_device();
  graphics_binding.queueFamilyIndex = device.get_queue_family_index();
  graphics_binding.queueIndex = device.get_queue_index();


  XrSessionCreateInfo session_create_info{ XR_TYPE_SESSION_CREATE_INFO };
  session_create_info.createFlags = 0;
  session_create_info.systemId = instance_vr.get_system_id();
  session_create_info.next = (void*)(&graphics_binding);

  OPENXR_CHECK_INSTANCE(
    xrCreateSession(instance_vr.get_instance(), &session_create_info, &session_),
    "Failed to create Session.",
    instance_vr.get_instance());

  GetConfigurationViews(view_configuration_type);
}

LavaSessionVR::~LavaSessionVR()
{
  OPENXR_CHECK_INSTANCE(xrDestroySession(session_), "Failed to destroy Session.", instance_vr_.get_instance());
}

void LavaSessionVR::GetConfigurationViews(XrViewConfigurationType view_configuration_type) {
  // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
  uint32_t view_configuration_count = 0;
  std::vector<XrViewConfigurationType> view_configurations = {};

  OPENXR_CHECK_INSTANCE(
    xrEnumerateViewConfigurations(instance_vr_.get_instance(), instance_vr_.get_system_id(), 0, &view_configuration_count, nullptr),
    "Failed to enumerate View Configurations.",
    instance_vr_.get_instance())

  view_configurations.resize(view_configuration_count);
  OPENXR_CHECK_INSTANCE(
    xrEnumerateViewConfigurations(instance_vr_.get_instance(), instance_vr_.get_system_id(), view_configuration_count, &view_configuration_count, view_configurations.data()),
    "Failed to enumerate View Configurations.",
    instance_vr_.get_instance())

  if (std::find(view_configurations.begin(), view_configurations.end(), view_configuration_type) != view_configurations.end()) {
    view_configuration_type_ = view_configuration_type;
  }

  if (view_configuration_type_ == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM) {
    std::cerr << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO." << std::endl;
    view_configuration_type_ = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
  }

  // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
  uint32_t view_configuration_view_count = 0;
  OPENXR_CHECK_INSTANCE(
    xrEnumerateViewConfigurationViews(instance_vr_.get_instance(), instance_vr_.get_system_id(), view_configuration_type_, 0, &view_configuration_view_count, nullptr),
    "Failed to enumerate ViewConfiguration Views.",
    instance_vr_.get_instance());

  view_configuration_views.resize(view_configuration_view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
  OPENXR_CHECK_INSTANCE(
    xrEnumerateViewConfigurationViews(instance_vr_.get_instance(), instance_vr_.get_system_id(), view_configuration_type_, view_configuration_view_count, &view_configuration_view_count, view_configuration_views.data()),
    "Failed to enumerate ViewConfiguration Views.",
    instance_vr_.get_instance());
}
