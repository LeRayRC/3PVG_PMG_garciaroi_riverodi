#include "engine/lava_device.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_surface.hpp"
#include "lava_vulkan_helpers.hpp"
#include "lava/openxr_common/OpenXRHelper.h"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"
#include "vr/lava_helpers_vr.hpp"

LavaDevice::LavaDevice(LavaInstance& instance, LavaSurface& surface){
	pickPhysicalDevice(instance, surface);
	createLogicalDevice(surface);
}

LavaDevice::LavaDevice(LavaInstance& instance, LavaInstanceVR& instance_vr, LavaBindingVR& binding) {
	pickPhysicalDevice(instance, instance_vr, binding);
	createLogicalDevice(instance_vr, binding);
}

LavaDevice::~LavaDevice(){
	vkDestroyDevice(device_, nullptr);
}

void LavaDevice::operator=(LavaDevice& device) {
	device_ = device.device_;
}

void LavaDevice::pickPhysicalDevice(LavaInstance& instance, LavaSurface& surface) {
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
#ifndef NDEBUG
		printf("Physical device is NULL");
#endif // !NDEBUG
		exit(-1);
	}
}

void LavaDevice::pickPhysicalDevice(LavaInstance& instance, 
	LavaInstanceVR& instance_vr, LavaBindingVR& binding) {
	// Physical Device
	uint32_t physicalDeviceCount = 0;
	std::vector<VkPhysicalDevice> physicalDevices;
	VULKAN_CHECK(vkEnumeratePhysicalDevices(instance.get_instance(), &physicalDeviceCount, nullptr), "Failed to enumerate PhysicalDevices.");
	physicalDevices.resize(physicalDeviceCount);
	VULKAN_CHECK(vkEnumeratePhysicalDevices(instance.get_instance(), &physicalDeviceCount, physicalDevices.data()), "Failed to enumerate PhysicalDevices.");

	VkPhysicalDevice physicalDeviceFromXR;

	OPENXR_CHECK_INSTANCE(
		binding.get_vulkan_graphics_device_binding()(instance_vr.get_instance(), instance_vr.get_system_id(), instance.get_instance(), &physicalDeviceFromXR),
		"Failed to get Graphics Device for Vulkan.",
		instance_vr.get_instance()
		);

	auto physicalDeviceFromXR_it = std::find(physicalDevices.begin(), physicalDevices.end(), physicalDeviceFromXR);
	if (physicalDeviceFromXR_it != physicalDevices.end()) {
		physical_device_ = *physicalDeviceFromXR_it;
	}
	else {
		std::cout << "ERROR: Vulkan: Failed to find PhysicalDevice for OpenXR." << std::endl;
		// Select the first available device.
		physical_device_ = physicalDevices[0];
	}
}

void LavaDevice::createLogicalDevice(LavaSurface& surface) {
	/*
	* Se tienen que rellenar diversas estructuras, la primera
	* indica el numero de colas que queremos crear
	*/
	QueueFamilyIndices indices = FindQueueFamilies(physical_device_, surface.get_surface());
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	float queuePriorities[] = { 1.0f , 1.0f};

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkQueueFamilyProperties familyProperties{};
		VkDeviceQueueCreateInfo queueCreateInfo{};
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamily, &familyProperties);
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 2;
		queueCreateInfo.pQueuePriorities = queuePriorities;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	/* Tambien hay que indicar los features que se quieren
	* activar de la GPU, de momento se han dejado en blanco
	* pero se modificaran mas adelante
	*/
	//Dynamic rendering
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature{
	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
	.dynamicRendering = VK_TRUE,
	};

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_feature_info;
	buffer_device_address_feature_info.bufferDeviceAddress = VK_TRUE;
	buffer_device_address_feature_info.bufferDeviceAddressCaptureReplay = VK_FALSE;
	buffer_device_address_feature_info.bufferDeviceAddressMultiDevice = VK_FALSE;
	buffer_device_address_feature_info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	buffer_device_address_feature_info.pNext = &dynamic_rendering_feature;

	//Habilitar indexacion de descriptores
	//VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {};
	//indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	//indexing_features.pNext = &buffer_device_address_feature_info;

	VkPhysicalDeviceFeatures2 device_features{};
	device_features.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	device_features.pNext = &buffer_device_address_feature_info;

	vkGetPhysicalDeviceFeatures2(physical_device_, &device_features);



	/*
	* Ahora se rellena la estructura para crear el dispositivo logico
	*/
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	//createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions_.size());
	createInfo.ppEnabledExtensionNames = required_device_extensions_.data();
	//Al tener la extension de synchronization2 se tiene que agregar una estructura extra
	VkPhysicalDeviceSynchronization2Features sync2Info{};
	sync2Info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync2Info.synchronization2 = VK_TRUE;

	createInfo.pNext = &sync2Info;
	sync2Info.pNext = &device_features;

	if (vkCreateDevice(physical_device_, &createInfo,
		nullptr, &device_) != VK_SUCCESS) {
		printf("Failed to create logical device");
		exit(-1);
	}

	/*
	Indica el indice de la cola que se va a seleccionar de la familia correspondiente
	0 graficos y presentacion
	1 transferencia
	*/
	vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphics_queue_);
	vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &present_queue_);
	vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 1, &transfer_queue_);
}

void LavaDevice::createLogicalDevice(LavaInstanceVR& instance_vr,LavaBindingVR& binding) {
	// Device
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyPropertiesCount, nullptr);
	queueFamilyProperties.resize(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs;
	std::vector<std::vector<float>> queuePriorities;
	queuePriorities.resize(queueFamilyProperties.size());
	deviceQueueCIs.resize(queueFamilyProperties.size());
	for (size_t i = 0; i < deviceQueueCIs.size(); i++) {
		for (size_t j = 0; j < queueFamilyProperties[i].queueCount; j++)
			queuePriorities[i].push_back(1.0f);

		deviceQueueCIs[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCIs[i].pNext = nullptr;
		deviceQueueCIs[i].flags = 0;
		deviceQueueCIs[i].queueFamilyIndex = static_cast<uint32_t>(i);
		deviceQueueCIs[i].queueCount = queueFamilyProperties[i].queueCount;
		deviceQueueCIs[i].pQueuePriorities = queuePriorities[i].data();

		if (BitwiseCheck(queueFamilyProperties[i].queueFlags, VkQueueFlags(VK_QUEUE_GRAPHICS_BIT)) && queue_family_index_ == 0xFFFFFFFF && queue_index_ == 0xFFFFFFFF) {
			queue_family_index_ = static_cast<uint32_t>(i);
			queue_index_ = 0;
		}
	}

	std::vector<const char*> activeDeviceExtensions{};


	uint32_t device_extension_count = 0;
	VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(physical_device_, 0, &device_extension_count, 0), "Failed to enumerate DeviceExtensionProperties.");
	std::vector<VkExtensionProperties> device_extension_properties;
	device_extension_properties.resize(device_extension_count);

	//VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	//	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,


	VULKAN_CHECK(vkEnumerateDeviceExtensionProperties(physical_device_, 0, &device_extension_count, device_extension_properties.data()), "Failed to enumerate DeviceExtensionProperties.");
	const std::vector<std::string>& openXrDeviceExtensionNames = GetDeviceExtensionsForOpenXR(instance_vr, binding);
	for (const std::string& requestExtension : openXrDeviceExtensionNames) {
		for (const VkExtensionProperties& extensionProperty : device_extension_properties) {
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				activeDeviceExtensions.push_back(requestExtension.c_str());
			break;
		}
	}

	for (auto extension : required_device_extensions_) {
		activeDeviceExtensions.push_back(extension);
	}


	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.dynamicRendering = VK_TRUE,
	};

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_feature_info;
	buffer_device_address_feature_info.bufferDeviceAddress = VK_TRUE;
	buffer_device_address_feature_info.bufferDeviceAddressCaptureReplay = VK_FALSE;
	buffer_device_address_feature_info.bufferDeviceAddressMultiDevice = VK_FALSE;
	buffer_device_address_feature_info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	buffer_device_address_feature_info.pNext = &dynamic_rendering_feature;

	VkPhysicalDeviceFeatures2 device_features{};
	device_features.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	device_features.pNext = &buffer_device_address_feature_info;

	vkGetPhysicalDeviceFeatures2(physical_device_, &device_features);


	VkDeviceCreateInfo deviceCI = {};
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.pNext = nullptr;
	deviceCI.flags = 0;
	deviceCI.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCIs.size());
	deviceCI.pQueueCreateInfos = deviceQueueCIs.data();
	deviceCI.enabledLayerCount = 0;
	deviceCI.ppEnabledLayerNames = nullptr;
	deviceCI.enabledExtensionCount = static_cast<uint32_t>(activeDeviceExtensions.size());
	deviceCI.ppEnabledExtensionNames = activeDeviceExtensions.data();


	VkPhysicalDeviceSynchronization2Features sync2Info{};
	sync2Info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync2Info.synchronization2 = VK_TRUE;

	deviceCI.pNext = &sync2Info;
	sync2Info.pNext = &device_features;

	VULKAN_CHECK(vkCreateDevice(physical_device_, &deviceCI, nullptr, &device_), "Failed to create Device.");

}