#include "lava_instance_vr.hpp"
#include "lava/openxr_common/DebugOutput.h"
#include "lava/openxr_common/OpenXRDebugUtils.h"
#include <vulkan/vulkan.h>

LavaInstanceVR::LavaInstanceVR() : api_type_{VULKAN}
{
  CreateInstance();
  CreateDebugMessenger();
  GetInstanceProperties();
  GetSystemID();
}

void LavaInstanceVR::CreateInstance() {
  XrApplicationInfo application_info;
  strncpy(application_info.applicationName, "LavaOpenXR", XR_MAX_APPLICATION_NAME_SIZE);
  application_info.applicationVersion = 1;
  strncpy(application_info.engineName, "LavaEngine", XR_MAX_ENGINE_NAME_SIZE);
  application_info.engineVersion = 1;
  application_info.apiVersion = XR_CURRENT_API_VERSION;


  instance_extensions_.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
  instance_extensions_.push_back(GetGraphicsAPIInstanceExtensionString(api_type_));

  //Check API Layers from the OpenXR Runtime
  uint32_t api_layer_count;
  std::vector<XrApiLayerProperties> api_layer_properties;
  OPENXR_CHECK_INSTANCE(
    xrEnumerateApiLayerProperties(0, &api_layer_count, nullptr),
    "Failed to enumerate ApiLayerProperties",
    instance_);

  api_layer_properties.resize(api_layer_count, { XR_TYPE_API_LAYER_PROPERTIES });
  OPENXR_CHECK_INSTANCE(
    xrEnumerateApiLayerProperties(api_layer_count, &api_layer_count, api_layer_properties.data()),
    "Failed to enumerate ApiLayerProperties.",
    instance_);

  // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
  for (auto& requestLayer : api_layers) {
    for (auto& layerProperty : api_layer_properties) {
      // strcmp returns 0 if the strings match.
      if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0) {
        continue;
      }
      else {
        active_api_Layers.push_back(requestLayer.c_str());
        break;
      }
    }
  }

  // Get all the Instance Extensions from the OpenXR instance.
  uint32_t extension_count = 0;
  std::vector<XrExtensionProperties> extension_properties;
  OPENXR_CHECK_INSTANCE(
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extension_count, nullptr),
    "Failed to enumerate InstanceExtensionProperties.",
    instance_);

  extension_properties.resize(extension_count, { XR_TYPE_EXTENSION_PROPERTIES });

  OPENXR_CHECK_INSTANCE(
    xrEnumerateInstanceExtensionProperties(nullptr, extension_count, &extension_count, extension_properties.data()),
    "Failed to enumerate InstanceExtensionProperties.",
    instance_);

  // Check the requested Instance Extensions against the ones from the OpenXR runtime.
  // If an extension is found add it to Active Instance Extensions.
  // Log error if the Instance Extension is not found.
  for (auto& requested_instance_extension : instance_extensions_) {
    bool found = false;
    for (auto& extension_property : extension_properties) {
      // strcmp returns 0 if the strings match.
      if (strcmp(requested_instance_extension.c_str(), extension_property.extensionName) != 0) {
        continue;
      }
      else {
        active_instance_extensions.push_back(requested_instance_extension.c_str());
        found = true;
        break;
      }
    }
    if (!found) {
      XR_TUT_LOG_ERROR("Failed to find OpenXR instance extension: " << requested_instance_extension);
    }
  }


  XrInstanceCreateInfo instance_CI{ XR_TYPE_INSTANCE_CREATE_INFO };
  instance_CI.createFlags = 0;
  instance_CI.applicationInfo = application_info;
  instance_CI.enabledApiLayerCount = static_cast<uint32_t>(active_api_Layers.size());
  instance_CI.enabledApiLayerNames = active_api_Layers.data();
  instance_CI.enabledExtensionCount = static_cast<uint32_t>(active_instance_extensions.size());
  instance_CI.enabledExtensionNames = active_instance_extensions.data();
  OPENXR_CHECK_INSTANCE(
    xrCreateInstance(&instance_CI, &instance_),
    "Failed to create Instance.",
    instance_);
}


LavaInstanceVR::~LavaInstanceVR()
{
  // Check that "XR_EXT_debug_utils" is in the active Instance Extensions before destroying the XrDebugUtilsMessengerEXT.
  if (debug_utils_messenger != XR_NULL_HANDLE) {
    DestroyOpenXRDebugUtilsMessenger(instance_, debug_utils_messenger);  // From OpenXRDebugUtils.h.
  }
  OPENXR_CHECK_INSTANCE(xrDestroyInstance(instance_), "Failed to destroy Instance.", instance_);
}

void LavaInstanceVR::CreateDebugMessenger(){
  // Check that "XR_EXT_debug_utils" is in the active Instance Extensions before creating an XrDebugUtilsMessengerEXT.
  if (IsStringInVector(active_instance_extensions, XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    debug_utils_messenger = CreateOpenXRDebugUtilsMessenger(instance_);  // From OpenXRDebugUtils.h.
  }
}

void LavaInstanceVR::GetInstanceProperties() {
  XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
  OPENXR_CHECK_INSTANCE(
    xrGetInstanceProperties(instance_, &instanceProperties),
    "Failed to get InstanceProperties.",
    instance_);

  XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - "
    << XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
    << XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
    << XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void LavaInstanceVR::GetSystemID() {
  // Get the XrSystemId from the instance and the supplied XrFormFactor.
  XrSystemGetInfo system_get_info{ XR_TYPE_SYSTEM_GET_INFO };
  system_get_info.formFactor = form_factor_;
  OPENXR_CHECK_INSTANCE(
    xrGetSystem(instance_, &system_get_info, &system_id_), "Failed to get SystemID.",
    instance_);

  // Get the System's properties for some general information about the hardware and the vendor.
  OPENXR_CHECK_INSTANCE(
    xrGetSystemProperties(instance_, system_id_, &system_properties_),
    "Failed to get SystemProperties.",
    instance_);
}
