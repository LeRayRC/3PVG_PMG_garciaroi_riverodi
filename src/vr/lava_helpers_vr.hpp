#ifndef __LAVA_HELPERS_VR_H__
#define __LAVA_HELPERS_VR_H__ 1


std::vector<std::string> GetInstanceExtensionsForOpenXR(
  class LavaInstanceVR& instance, class LavaBindingVR& binding);

std::vector<std::string> GetDeviceExtensionsForOpenXR(class LavaInstanceVR& instance,
  class LavaBindingVR& binding);


#endif // !__LAVA_HELPERS_VR_H__
