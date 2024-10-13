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
#ifndef  __LAVA_ENGINE_
#define  __LAVA_ENGINE_ 1

#include "lava_types.hpp"
#include "lava_vulkan_helpers.hpp"
#include "lava_descriptors.hpp"
#include "engine/lava_instance.hpp"
#include "lava_window.hpp"
#include "engine/lava_surface.hpp"
#include "engine/lava_device.hpp"


struct DeletionQueue {
	//TO FIX -> multiples vector for every kind of Vulkan handle
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)();
		}
		deletors.clear();
	}
};

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;
	VkSemaphore swap_chain_semaphore; //GPU <-> GPU
	VkSemaphore render_semaphore; //GPU <-> GPU
	VkFence render_fence; // CPU <-> GPU
	DeletionQueue deletion_queue;
};

//Numero de buffers en paralelo (double-buffering)
constexpr unsigned int FRAME_OVERLAP = 2;

class LavaEngine {
public:

	/**
	* @brief Default constructor
	*/
	LavaEngine();

	/**
	* @brief Default constructor
	* 
	* @param window_width Desire width for the new window
	* @param window_height Desire height for the new window
	*/
	LavaEngine(unsigned int window_width, unsigned int window_height);

	/**
	* @brief Default destructor(destroy all resources incluying the window)
	*/
	~LavaEngine();

	/**
	* @brief Return the engine's intance
	*/
	//inline static Engine& Get() { return *loaded_engine; }

	//Engine* loaded_engine;

	bool is_initialized_ = false;
	bool stop_rendering = false;

	DeletionQueue main_deletion_queue_;
	VmaAllocator allocator_;
	//Recursos para dibujar fuera del swap chain
	AllocatedImage draw_image_;
	VkExtent2D draw_extent_;

	//GLFWwindow* window_;
	LavaWindow window_;
	LavaInstance instance_;
	LavaSurface surface_;
	LavaDevice device_;
	
	VkExtent2D window_extent_;
	//VkInstance instance_;
	VkDebugUtilsMessengerEXT debug_messenger_;
	//VkDevice device_;
	//VkPhysicalDevice physical_device_;
	//VkQueue graphics_queue_;
	//VkQueue present_queue_;
	//VkSurfaceKHR surface_;
	VkSwapchainKHR swap_chain_;
	std::vector<VkImage> swap_chain_images_;
	std::vector<VkImageView> swap_chain_image_views_;
	VkFormat swap_chain_image_format_;
	VkRenderPass render_pass_;
	//VkPipelineLayout pipeline_layout_;	
	//VkPipeline graphics_pipeline_;

	FrameData frames_[FRAME_OVERLAP];
	uint64_t  frame_number_ = 0;
	FrameData& getCurrentFrame() { return frames_[frame_number_ % FRAME_OVERLAP]; };

	DescriptorAllocator global_descriptor_allocator_;
	VkDescriptorSet draw_image_descriptor_set_;
	VkDescriptorSetLayout draw_image_descriptor_set_layout_;

	VkPipeline gradient_pipeline_;

	//Immediate Submit communication
	VkFence immediate_fence;
	VkCommandBuffer immediate_command_buffer;
	VkCommandPool immediate_command_pool;

	DescriptorAllocator imgui_descriptor_alloc;

	//Differents Effects
	std::vector<ComputeEffect> backgroundEffects;
	int currentBackgroundEffect{ 0 };

	

	void init();
	void initVulkan();
	void mainLoop();
	//void pickPhysicalDevice();
	//bool isDeviceSuitable(VkPhysicalDevice device);
	//void createLogicalDevice();
	void setupDebugMessenger();
	void createSwapChain();
	void createImageViews();
	void createCommandPool();
	void createSyncObjects();
	void draw();
	void drawBackground(VkCommandBuffer command_buffer);
	void drawBackgroundImGui(VkCommandBuffer command_buffer);
	void DrawGeometry(VkCommandBuffer command_buffer);
	
	void createAllocator();
	void createDescriptors();

	VkInstance get_instance() const;
	GLFWwindow* get_window() const;
	VkSurfaceKHR get_surface() const;

///////////////PIPELINES/////////

	void createPipelines();
	void createBackgroundPipelines();
	void createBackgroundPipelinesImGui();

	//Not use right now
	VkPipelineLayout _trianglePipelineLayout;
	VkPipeline _trianglePipeline;
	void createTrianglePipeline();
	//

	VkPipelineLayout _meshPipelineLayout;
	VkPipeline _meshPipeline;
	GPUMeshBuffers rectangle;
	void createMeshPipeline();
	void initDefaultData();

////////////////////////////////

//////////////MESHES////////////

	AllocatedBuffer createBuffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);

	void destroyBuffer(const AllocatedBuffer& buffer);

	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

////////////////////////////////

	void initImgui();
	void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
	void immediate_submit(std::function<void(VkCommandBuffer)>&& function);

private:
	/**
	* @brief Copy constructor(never use)
	*/
	LavaEngine(const LavaEngine& obj) = delete;

	/**
	* @brief Assign operator(never use)
	*/
	LavaEngine& operator=(const LavaEngine& obj) = delete;

	/**
	* @brief Move constructor(never use)
	*/
	LavaEngine(LavaEngine&& obj) = delete;

	/**
	* @brief Move Assign operator(never use)
	*/
	LavaEngine& operator=(LavaEngine& obj) = delete;

};
#endif // ! __CUSTOM_ENGINE_
