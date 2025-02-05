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
#include <mutex>
#include "engine/lava_image.hpp"


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

	//DeletionQueue main_deletion_queue_;
	GlobalSceneData global_scene_data_;
	CameraParameters camera_parameters_;
	static std::vector<std::function<void()>> end_frame_callbacks;

	LavaWindow window_;
	LavaInstance instance_;
	LavaSurface surface_;
	LavaDevice device_;
	VkExtent2D window_extent_;
	LavaAllocator allocator_;
	LavaSwapChain swap_chain_;
	LavaFrameData frame_data_;
	LavaInmediateCommunication inmediate_communication;
	
	LavaDescriptorManager global_descriptor_allocator_;
	std::unique_ptr<LavaBuffer> global_data_buffer_;
	VkDescriptorSetLayout global_descriptor_set_layout_;
	VkDescriptorSetLayout global_lights_descriptor_set_layout_;
	VkDescriptorSetLayout global_pbr_descriptor_set_layout_;
	VkDescriptorSet global_descriptor_set_;

	std::shared_ptr<LavaImage> default_texture_image_pink;
	std::shared_ptr<LavaImage> default_texture_image_white;
	std::shared_ptr<LavaImage> default_texture_image_black;

	std::chrono::steady_clock::time_point chrono_now_;
	std::chrono::steady_clock::time_point chrono_last_update_;

	static const unsigned int kMaxLights = 100;

	void allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector);
	void update_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector,
		std::vector<std::optional<struct TransformComponent>>& transform_vector);



	void updateMainCamera(struct CameraComponent* camera_component,
		struct TransformComponent* camera_tr);

	std::mutex queue_mutex_;
	double dt_;

	DescriptorAllocator imgui_descriptor_alloc;
	
	//draw stuff
	uint32_t swap_chain_image_index;
	VkCommandBuffer commandBuffer;

	bool shouldClose() { return glfwWindowShouldClose(get_window()); }
	void beginFrame();
	void endFrame();
	void clearWindow();
	void mainLoop();
	void render();
	void pollEvents() { glfwPollEvents(); }
	virtual void renderImgui();
	//void drawMeshes(VkCommandBuffer command_buffer);
	std::shared_ptr<class LavaMesh> addMesh(MeshProperties prop);

	VkInstance get_instance() const;
	GLFWwindow* get_window() const;
	VkSurfaceKHR get_surface() const;

	std::vector<std::shared_ptr<LavaMesh>> meshes_;
	
	void setDynamicViewportAndScissor();
	void initGlobalData();
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
