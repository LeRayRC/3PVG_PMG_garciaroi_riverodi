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
#include "lava/common/lava_engine_base.hpp"
#include "lava/window/lava_window.hpp"
#include "lava/engine/lava_descriptors.hpp"

class LavaEngine : public  LavaEngineBase{
public:

	/**
	* @brief Default constructor
	*/
	LavaEngine(unsigned int window_width = 1280, unsigned int window_height = 720);

	/**
	* @brief Default destructor(destroy all resources incluying the window)
	*/
	~LavaEngine();

	/**
	* @brief Return the engine's intance
	*/
	//inline static Engine& Get() { return *loaded_engine; }

	//Engine* loaded_engine;
	bool stop_rendering = false;

	//DeletionQueue main_deletion_queue_;
	
	static std::vector<std::function<void()>> end_frame_callbacks;

	LavaWindow window_;
	std::unique_ptr<class LavaInstance> instance_;
	std::unique_ptr<class LavaSurface> surface_;
	std::unique_ptr<class LavaDevice> device_;
	std::unique_ptr<class LavaAllocator> allocator_;

	VkExtent2D window_extent_;

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

	GlobalSceneData global_scene_data_;
	
	//draw stuff
	uint32_t swap_chain_image_index;
	VkCommandBuffer commandBuffer;

	bool shouldClose() override { return glfwWindowShouldClose(get_window()); }
	void beginFrame() override;
	void endFrame() override;
	void clearWindow() override;
	void pollEvents() override { glfwPollEvents(); }
	virtual void renderImgui();

	GLFWwindow* get_window() const;
	VkSurfaceKHR get_surface() const;

	DescriptorAllocator imgui_descriptor_alloc;

	void setDynamicViewportAndScissor(const VkExtent2D& extend);
	void initGlobalData();
	void initImgui();
	void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
	void immediate_submit(std::function<void(VkCommandBuffer)>&& function);
	void updateMainCamera() override;

	VkInstance get_instance();
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
