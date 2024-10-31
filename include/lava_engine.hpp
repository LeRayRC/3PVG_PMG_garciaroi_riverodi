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
#include "engine/lava_swap_chain.hpp"
#include "engine/lava_allocator.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_inmediate_communication.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_material.hpp"
#include "engine/lava_mesh.hpp"
#include "engine/lava_descriptor_manager.hpp"


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

	LavaWindow window_;
	LavaInstance instance_;
	LavaSurface surface_;
	LavaDevice device_;
	VkExtent2D window_extent_;
	LavaAllocator allocator_;
	LavaSwapChain swap_chain_;
	LavaFrameData frame_data_;
	LavaInmediateCommunication inmediate_communication;
	
	
	DescriptorAllocator global_descriptor_allocator_;
	VkDescriptorSet draw_image_descriptor_set_;
	VkDescriptorSetLayout generic_descriptor_set_layout_;

	//VkPipeline gradient_pipeline_;

	DescriptorAllocator imgui_descriptor_alloc;

	//Differents Effects
	std::vector<ComputeEffect> backgroundEffects;
	int currentBackgroundEffect{ 0 };

	
	void init();
	void initVulkan();
	void mainLoop();
	
	void draw();
	//void drawBackground(VkCommandBuffer command_buffer);
	//void drawBackgroundImGui(VkCommandBuffer command_buffer);
	//void DrawGeometry(VkCommandBuffer command_buffer);
	//void DrawGeometryWithProperties(VkCommandBuffer command_buffer);
	void drawMeshes(VkCommandBuffer command_buffer);
	std::shared_ptr<class LavaMesh> addMesh(MeshProperties prop);
	void createDescriptors();

	VkInstance get_instance() const;
	GLFWwindow* get_window() const;
	VkSurfaceKHR get_surface() const;

	void createBackgroundPipelines();
	void createBackgroundPipelinesImGui();

	std::vector<std::shared_ptr<LavaMesh>> meshes_;

	AllocatedBuffer createBuffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
	void destroyBuffer(const AllocatedBuffer& buffer);
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
