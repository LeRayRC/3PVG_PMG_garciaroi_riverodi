#ifndef __CUSTOM_VULKAN_HELPERS_H
#define __CUSTOM_VULKAN_HELPERS_H 1

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

//Se encarga de comprobar si todas las validation layers solicitadas 
//estan disponibles dentro de la instancia
bool checkValidationLayerSupport(const std::vector<const char*> validationLayers);

bool isDeviceSuitable(VkPhysicalDevice device);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);

#endif // !__CUSTOM_VULKAN_HELPERS_H
