#ifndef  __CUSTOM_ENGINE_
#define  __CUSTOM_ENGINE_ 1

#include "custom_types.hpp"

class Engine {
public:

	//Singleton
	static Engine& Get();

  bool is_initialized_ = false;
  int frame_number_ = 0;
  bool stop_rendering = false;

  GLFWwindow* window_												= nullptr;
	// TO FIX -> Hardcoded window size
	VkExtent2D window_extent_									= { 1280,720 };
	VkInstance instance_											=	VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device_					= VK_NULL_HANDLE;
	VkDevice device_													= VK_NULL_HANDLE;
	VkQueue graphics_queue_										= VK_NULL_HANDLE;
	VkQueue present_queue_										= VK_NULL_HANDLE;
	VkSurfaceKHR surface_											= VK_NULL_HANDLE;
	VkSwapchainKHR swapChain									= VK_NULL_HANDLE;
	std::vector<VkImage> swap_chain_images_;
	std::vector<VkImageView> swap_chain_image_views_;
	VkFormat swap_chain_image_format_					= VK_FORMAT_UNDEFINED;
	VkRenderPass render_pass_									= VK_NULL_HANDLE;
	VkPipelineLayout pipeline_layout_					= VK_NULL_HANDLE;	
	VkPipeline graphics_pipeline_							= VK_NULL_HANDLE;

  void init();
	void initWindow();
  void cleanUp();
  void initVulkan();
	void mainLoop();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createInstance();
	void createLogicalDevice();
	void setupDebugMessenger();
	void createSurface();
	void createSwapChain();
	void createImageViews();
	void createGraphicsPipeline();
	void createRenderPass();
	Engine();
	~Engine();
private:
};
#endif // ! __CUSTOM_ENGINE_
