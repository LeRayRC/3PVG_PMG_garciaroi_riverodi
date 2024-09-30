#ifndef __CUSTOM_VULKAN_HELPERS_H
#define __CUSTOM_VULKAN_HELPERS_H 1

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <optional>

struct QueueFamilyIndices {
	//Familia para lanzar comandos de graficos
	std::optional<uint32_t> graphicsFamily;
	//Familia para lanzar comandos de presentacion en superficie
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && 
			presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

//Se encarga de comprobar si todas las validation layers solicitadas 
//estan disponibles dentro de la instancia
bool checkValidationLayerSupport(const std::vector<const char*> validationLayers);

bool checkDeviceExtensionsSupport(VkPhysicalDevice device,
	std::vector<const char*> requiredExtensions);

std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
	VkSurfaceKHR surface);


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
	VkSurfaceKHR surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const
	std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR chooseSwapPresentMode(const
	std::vector<VkPresentModeKHR>& availablePresentModes);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&
	capabilities, GLFWwindow* window);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);

std::vector<char> readFile(const std::string& filename);

VkShaderModule createShaderModule(VkDevice device,const std::vector<char>& code);

#endif // !__CUSTOM_VULKAN_HELPERS_H
