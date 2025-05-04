#include "vr/lava_helpers_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"

std::vector<std::string> GetInstanceExtensionsForOpenXR(LavaInstanceVR& instance, LavaBindingVR& binding) {
  uint32_t extension_names_size = 0;
  OPENXR_CHECK_INSTANCE(
    binding.get_vulkan_instance_extensions_binding()(instance.get_instance(), instance.get_system_id(), 0, &extension_names_size, nullptr),
    "Failed to get Vulkan Instance Extensions.",
    instance.get_instance());

  std::vector<char> extension_names(extension_names_size);

  OPENXR_CHECK_INSTANCE(
    binding.get_vulkan_instance_extensions_binding()(instance.get_instance(), instance.get_system_id(), extension_names_size, &extension_names_size, extension_names.data()),
    "Failed to get Vulkan Instance Extensions.",
    instance.get_instance());

  std::stringstream streamData(extension_names.data());
  std::vector<std::string> extensions;
  std::string extension;
  while (std::getline(streamData, extension, ' ')) {
    extensions.push_back(extension);
  }
  return extensions;
}


std::vector<std::string> GetDeviceExtensionsForOpenXR(LavaInstanceVR& instance,LavaBindingVR& binding) {
  uint32_t extension_names_size = 0;

  OPENXR_CHECK_INSTANCE(
    binding.get_vulkan_device_extensions_binding()(instance.get_instance(), instance.get_system_id(), 0, &extension_names_size, nullptr),
    "Failed to get Vulkan Device Extensions.",
    instance.get_instance());

  std::vector<char> extension_names(extension_names_size);

  OPENXR_CHECK_INSTANCE(
    binding.get_vulkan_device_extensions_binding()(instance.get_instance(), instance.get_system_id(), extension_names_size, &extension_names_size, extension_names.data()),
    "Failed to get Vulkan Device Extensions.",
    instance.get_instance());

  std::stringstream streamData(extension_names.data());
  std::vector<std::string> extensions;
  std::string extension;
  while (std::getline(streamData, extension, ' ')) {
    extensions.push_back(extension);
  }
  return extensions;
}