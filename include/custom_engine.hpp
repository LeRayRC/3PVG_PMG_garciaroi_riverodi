/**
 * @file custom_engine.hpp
 * @author ???
 * @brief Custom Engine's header file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */
#ifndef  __CUSTOM_ENGINE_
#define  __CUSTOM_ENGINE_ 1

#include "custom_types.hpp"

class Engine {
public:

	/**
	* @brief Default constructor
	*/
	Engine();

	/**
	* @brief Default constructor
	* 
	* @param window_width Desire width for the new window
	* @param window_height Desire height for the new window
	*/
	Engine(unsigned int window_width, unsigned int window_height);

	/**
	* @brief Default destructor(destroy all resources incluying the window)
	*/
	~Engine();

	/**
	* @brief Return the engine's intance
	*/
	inline static Engine& Get() { return *loaded_engine; }

	bool is_initialized_ = false;
	int frame_number_ = 0;
	bool stop_rendering = false;

	GLFWwindow* window_				;
	// TO FIX -> Hardcoded window size
	VkExtent2D window_extent_;
	VkInstance instance_;
	VkDebugUtilsMessengerEXT debug_messenger_;
	VkPhysicalDevice physical_device_;
	VkDevice device_;
	VkQueue graphics_queue_;
	VkQueue present_queue_;
	VkSurfaceKHR surface_;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swap_chain_images_;
	std::vector<VkImageView> swap_chain_image_views_;
	VkFormat swap_chain_image_format_;
	VkRenderPass render_pass_;
	VkPipelineLayout pipeline_layout_;	
	VkPipeline graphics_pipeline_;

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

private:
	/**
	* @brief Copy constructor(never use)
	*/
	Engine(const Engine& obj) {};

	/**
	* @brief Assign operator(never use)
	*/
	Engine& operator=(const Engine& obj){};

	/**
	* @brief Move constructor(never use)
	*/
	Engine(Engine&& obj) {};

	/**
	* @brief Move Assign operator(never use)
	*/
	Engine& operator=(Engine& obj) {};

	static Engine* loaded_engine;
};
#endif // ! __CUSTOM_ENGINE_
