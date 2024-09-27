#include "custom_vulkan_helpers.hpp"

#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << "Validation layer " << pCallbackData->pMessage << std::endl;
	}

	return VK_FALSE;
}


bool checkValidationLayerSupport(const std::vector<const char*> validationLayers) {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}


std::vector<const char*> getRequiredExtensions(bool enableValidationLayers) {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions,
		glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

//Esta funcion es un tanto peculiar ya que actua de proxy para utilizar la 
// funcion vkCreateDebugUtilsMessengerEXT ya que al formar parte de una extension
// no se carga automaticamente asi que hay que hacerlo a través de la funcion
// vkGetInstanceProcAddr que nos da la direccion de memoria donde se encuentra 
// esa función exactamente
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance,
			"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//Se necesita el mismo planteamiento para destruir este objeto, a traves 
// de la funcion vkDestroyDebugUtilsMessengerEXT
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance,
			"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


bool isDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	QueueFamilyIndices indices = findQueueFamilies(device);
	//Para que una GPU sea valida tiene que ser dedicada. Se pueden realizar las
	// comprobaciones que se quieran en base a las properties o las features
	// p.e se puede comprobar si la grafica permite usar geometry shaders
	// return deviceFeatures.geometryShader
	//return deviceProperties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	//No es muy importante para el inicio del motor tener esto bien configurado
	//Por conveniencia vamos a devolver true siempre y ya se cambiara
	//El tutorial recomiendo asociar una puntuacion a cada grafica del sistema 
	//para al menos seleccionar una
	// https://vulkan-tutorial.com/resources/vulkan_tutorial_en.pdf pag 60
	return indices.isComplete();
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
		queueFamilies.data());
	
	//Dentro de las colas que existen dentro de la grafica
	//Se pueden realizar las comprobaciones que se deseen
	//En este caso se ha decicido encontrar una que soporte 
	//los comandos graficos
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
		i++;
	}
	
	return indices;
}