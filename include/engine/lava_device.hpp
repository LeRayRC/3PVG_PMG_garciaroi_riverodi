#ifndef __LAVA_DEVICE_H_ 
#define __LAVA_DEVICE_H_ 1

#include "lava_types.hpp"
class LavaInstance;
class LavaSurface;

class LavaDevice
{
public:
	LavaDevice(LavaInstance& instance, LavaSurface& surface);
	~LavaDevice();

	void operator=(LavaDevice& device);

	VkPhysicalDevice get_physical_device() const{
		return physical_device_;
	}

	VkDevice get_device() const {
		return device_;
	}

	VkQueue get_graphics_queue() const {
		return graphics_queue_;
	}

	VkQueue get_present_queue() const {
		return present_queue_;
	}

	const std::vector<const char*> required_device_extensions_ = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	};

private:
	VkPhysicalDevice physical_device_;
	VkDevice device_;
	VkQueue graphics_queue_;
	VkQueue present_queue_;


	void createLogicalDevice(LavaSurface& surface);
	void pickPhysicalDevice(LavaInstance& instance, LavaSurface& surface);


};






#endif // !__LAVA_DEVICE_H_ 
