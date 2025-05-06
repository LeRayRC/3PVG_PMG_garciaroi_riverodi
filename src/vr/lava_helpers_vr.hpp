#ifndef __LAVA_HELPERS_VR_H__
#define __LAVA_HELPERS_VR_H__ 1



std::vector<std::string> GetInstanceExtensionsForOpenXR(
  class LavaInstanceVR& instance, class LavaBindingVR& binding);

std::vector<std::string> GetDeviceExtensionsForOpenXR(class LavaInstanceVR& instance,
  class LavaBindingVR& binding);

int64_t SelectDepthSwapchainFormat(const std::vector<int64_t>& formats);

const std::vector<int64_t> GetSupportedDepthSwapchainFormats();

int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& formats);
const std::vector<int64_t> GetSupportedColorSwapchainFormats();

#endif // !__LAVA_HELPERS_VR_H__
