#include "vr/lava_helpers_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"
#include "lava/openxr_common/OpenXRHelper.h"

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

int64_t SelectDepthSwapchainFormat(const std::vector<int64_t>& formats) {
  const std::vector<int64_t>& supportSwapchainFormats = GetSupportedDepthSwapchainFormats();

  const std::vector<int64_t>::const_iterator& swapchainFormatIt = std::find_first_of(formats.begin(), formats.end(),
    std::begin(supportSwapchainFormats), std::end(supportSwapchainFormats));
  if (swapchainFormatIt == formats.end()) {
    std::cout << "ERROR: Unable to find supported Depth Swapchain Format" << std::endl;
    DEBUG_BREAK;
    return 0;
  }

  return *swapchainFormatIt;
}

const std::vector<int64_t> GetSupportedDepthSwapchainFormats() {
  return {
      VK_FORMAT_D32_SFLOAT };//,
      //VK_FORMAT_D16_UNORM };
}

int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& formats) {
  const std::vector<int64_t>& supportSwapchainFormats = GetSupportedColorSwapchainFormats();

  const std::vector<int64_t>::const_iterator& swapchainFormatIt = std::find_first_of(formats.begin(), formats.end(),
    std::begin(supportSwapchainFormats), std::end(supportSwapchainFormats));
  if (swapchainFormatIt == formats.end()) {
    std::cout << "ERROR: Unable to find supported Color Swapchain Format" << std::endl;
    DEBUG_BREAK;
    return 0;
  }

  return *swapchainFormatIt;
}

const std::vector<int64_t> GetSupportedColorSwapchainFormats() {
  return {
      VK_FORMAT_B8G8R8A8_SRGB,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_FORMAT_B8G8R8A8_UNORM,
      VK_FORMAT_R8G8B8A8_UNORM };
}
