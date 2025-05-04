#ifndef __LAVA_DEVICE_H_ 
#define __LAVA_DEVICE_H_ 1

#include "lava/common/lava_types.hpp"
class LavaInstance;
class LavaSurface;

class LavaDevice
{
public:
	//GLFW Constructor
	LavaDevice(LavaInstance& instance, LavaSurface& surface);
	//VR Constructor
	LavaDevice(LavaInstance& instance, class LavaInstanceVR& instance_vr, class LavaBindingVR& binding);
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

	VkQueue get_transfer_queue() const {
		return transfer_queue_;
	}

	uint32_t get_queue_family_index() {
		return queue_family_index_;
	}

	uint32_t get_queue_index() {
		return queue_index_;
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
	VkQueue transfer_queue_;

	//VR Usage
	uint32_t queue_family_index_ = 0xFFFFFFFF;
	uint32_t queue_index_ = 0xFFFFFFFF;


	void createLogicalDevice(LavaSurface& surface);
	void createLogicalDevice(class LavaInstanceVR& instance_vr, 
		class LavaBindingVR& binding);

	void pickPhysicalDevice(LavaInstance& instance, LavaSurface& surface);
	void pickPhysicalDevice(LavaInstance& instance,
		class LavaInstanceVR& instance_vr, class LavaBindingVR& binding);

};






#endif // !__LAVA_DEVICE_H_ 
