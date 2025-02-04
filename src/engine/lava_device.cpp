#include "engine/lava_device.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_surface.hpp"
#include "lava_vulkan_helpers.hpp"

LavaDevice::LavaDevice(LavaInstance& instance, LavaSurface& surface){
	pickPhysicalDevice(instance, surface);
	createLogicalDevice(surface);
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
	VkPhysicalDeviceDescriptorIndexingFeatures indexing_features = {};
	indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	indexing_features.pNext = &buffer_device_address_feature_info;

	VkPhysicalDeviceFeatures2 device_features{};
	device_features.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	device_features.pNext = &buffer_device_address_feature_info;

	vkGetPhysicalDeviceFeatures2(physical_device_, &device_features);



	// 2. Verificar si la característica está soportada
	//if (!indexing_features.descriptorBindingSampledImageUpdateAfterBind) {
	//	throw std::runtime_error("descriptorBindingSampledImageUpdateAfterBind not supported.");
	//}

	

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

	VkPhysicalDeviceDescriptorIndexingFeatures enabled_indexing_features = {};
	enabled_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	enabled_indexing_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

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