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

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;
	VkSemaphore swap_chain_semaphore; //GPU <-> GPU
	VkSemaphore render_semaphore; //GPU <-> GPU
	VkFence render_fence; // CPU <-> GPU
};

//Numero de buffers en paralelo (double-buffering)
constexpr unsigned int FRAME_OVERLAP = 2;

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
	//inline static Engine& Get() { return *loaded_engine; }

	//Engine* loaded_engine;

	bool is_initialized_ = false;
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
	VkSwapchainKHR swap_chain_;
	std::vector<VkImage> swap_chain_images_;
	std::vector<VkImageView> swap_chain_image_views_;
	VkFormat swap_chain_image_format_;
	VkRenderPass render_pass_;
	VkPipelineLayout pipeline_layout_;	
	VkPipeline graphics_pipeline_;

	FrameData frames_[FRAME_OVERLAP];
	int frame_number_ = 0;
	FrameData& getCurrentFrame() { return frames_[frame_number_ % FRAME_OVERLAP]; };

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
	void createCommandPool();
	void createSyncObjects();
	void draw();
	void transitionImage(VkCommandBuffer cmd, VkImage image, 
		VkImageLayout currentLayout, VkImageLayout newLayout);
	VkImageSubresourceRange  imageSubresourceRange(VkImageAspectFlags aspectMask);

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
};
#endif // ! __CUSTOM_ENGINE_
