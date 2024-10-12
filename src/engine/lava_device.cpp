#include "engine/lava_device.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_surface.hpp"
#include "lava_vulkan_helpers.hpp"

LavaDevice::LavaDevice(LavaInstance& instance, LavaSurface surface){
	uint32_t deviceCount = 0;
	//Primero se obtienen la cantidad de GPUs disponibles en el equipo
	vkEnumeratePhysicalDevices(instance.get_instance(), &deviceCount, nullptr);
	if (deviceCount == 0) {
#ifndef NDEBUG
		printf("There are no physical devices available");
#endif // !NDEBUG
		exit(-1);
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	//Ahora se meten los datos de las GPUs en el vector devices
	vkEnumeratePhysicalDevices(instance.get_instance(), &deviceCount, devices.data());
	for (const auto& device : devices) {
		if (IsDeviceSuitable(device, surface.get_surface(), required_device_extensions_)) {
			physical_device_ = device;
		}
	}
	if (physical_device_ == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find suitable GPU");
	}
}

LavaDevice::~LavaDevice(){

}