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

#include "lava/common/lava_types.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "lava/window/lava_window.hpp"
#include "lava/engine/lava_descriptors.hpp"


 // push constants for our mesh object draws
struct GPUDrawPushConstants {
	glm::mat4 world_matrix;
	VkDeviceAddress vertex_buffer;
};

struct GlobalSceneData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
	glm::vec3 ambientColor;
	int padding1;
	glm::vec3 cameraPos;
	int padding2;
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

	//DeletionQueue main_deletion_queue_;
	GlobalSceneData global_scene_data_;
	static std::vector<std::function<void()>> end_frame_callbacks;

	LavaWindow window_;
	std::unique_ptr<class LavaInstance> instance_;
	std::unique_ptr<class LavaSurface> surface_;
	std::unique_ptr<class LavaDevice> device_;
	VkExtent2D window_extent_;
	std::unique_ptr<class LavaAllocator> allocator_;
	std::unique_ptr<class LavaSwapChain> swap_chain_;
	std::unique_ptr<class LavaFrameData> frame_data_;
	std::unique_ptr<class LavaInmediateCommunication> inmediate_communication;
	
	std::unique_ptr<class LavaDescriptorManager> global_descriptor_allocator_;
	std::unique_ptr<class LavaBuffer> global_data_buffer_;
	VkDescriptorSetLayout global_descriptor_set_layout_;
	VkDescriptorSetLayout global_lights_descriptor_set_layout_;
	VkDescriptorSetLayout global_pbr_descriptor_set_layout_;
	VkDescriptorSet global_descriptor_set_;

	std::shared_ptr<class LavaImage> default_texture_image_pink;
	std::shared_ptr<class LavaImage> default_texture_image_white;
	std::shared_ptr<class LavaImage> default_texture_image_black;

	std::chrono::steady_clock::time_point chrono_now_;
	std::chrono::steady_clock::time_point chrono_last_update_;

	void updateMainCamera(struct CameraComponent* camera_component,
		struct TransformComponent* camera_tr);

	double dt_;

	DescriptorAllocator imgui_descriptor_alloc;
	
	//draw stuff
	uint32_t swap_chain_image_index;
	VkCommandBuffer commandBuffer;

	bool shouldClose() { return glfwWindowShouldClose(get_window()); }
	void beginFrame();
	void endFrame();
	void clearWindow();
	void pollEvents() { glfwPollEvents(); }
	virtual void renderImgui();

	VkInstance get_instance() const;
	GLFWwindow* get_window() const;
	VkSurfaceKHR get_surface() const;

	
	void setDynamicViewportAndScissor(const VkExtent2D& extend);
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
