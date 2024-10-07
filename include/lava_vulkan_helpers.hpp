#ifndef __LAVA_VULKAN_HELPERS_H
#define __LAVA_VULKAN_HELPERS_H 1

#include "lava_types.hpp"

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

struct AllocatedImage {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

//Se encarga de comprobar si todas las validation layers solicitadas 
//estan disponibles dentro de la instancia
bool CheckValidationLayerSupport(const std::vector<const char*> validationLayers);

bool CheckDeviceExtensionsSupport(VkPhysicalDevice device,
	std::vector<const char*> requiredExtensions);

std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers);

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
	VkSurfaceKHR surface);


SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
	VkSurfaceKHR surface);

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const
	std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR ChooseSwapPresentMode(const
	std::vector<VkPresentModeKHR>& availablePresentModes);

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR&
	capabilities, GLFWwindow* window);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);



void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, 
	VkExtent2D srcSize, VkExtent2D dstSize);

void TransitionImage(VkCommandBuffer cmd, VkImage image,
	VkImageLayout currentLayout, VkImageLayout newLayout);

VkImageSubresourceRange  ImageSubresourceRange(VkImageAspectFlags aspectMask);

bool LoadShader(const std::string& file_path,
	VkDevice device,
	VkShaderModule* out_shader_module);
VkShaderModule CreateShaderModule(VkDevice device,const std::vector<char>& code);
std::vector<char> ReadFile(const std::string& filename);

#endif // !__CUSTOM_VULKAN_HELPERS_H
