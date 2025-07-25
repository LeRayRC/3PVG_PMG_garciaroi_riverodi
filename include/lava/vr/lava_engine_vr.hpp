#ifndef __LAVA_ENGINE_VR_H__
#define __LAVA_ENGINE_VR_H__ 1

//#ifndef XR_USE_GRAPHICS_API_VULKAN
//#define XR_USE_GRAPHICS_API_VULKAN 1
//#endif

#include "lava/common/lava_engine_base.hpp"
#include "lava/vr/lava_data_structures_vr.hpp"


struct Viewport {
	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};
struct Offset2D {
	int32_t x;
	int32_t y;
};
struct Extent2D {
	uint32_t width;
	uint32_t height;
};
struct Rect2D {
	Offset2D offset;
	Extent2D extent;
};

class LavaEngineVR : public LavaEngineBase
{
public:
	LavaEngineVR(struct XrPosef reference_pose);
	~LavaEngineVR();

	std::unique_ptr<class LavaInstanceVR> instance_vr_;
	std::unique_ptr<class LavaBindingVR> binding_;
	std::unique_ptr<class LavaInstance> instance_vulkan_;
	std::unique_ptr<class LavaDevice> device_;
	std::unique_ptr<class LavaSessionVR> session_;
	std::unique_ptr<class LavaSwapchainVR> swapchain_;
	std::unique_ptr<class LavaBlendSpaceVR> blend_space_;
	std::unique_ptr<class LavaInmediateCommunication> inmediate_communication;

	std::unique_ptr<class LavaAllocator> allocator_;
	std::unique_ptr<class LavaFrameData> frame_data_[2];

	std::unique_ptr<class LavaDescriptorManager> global_descriptor_allocator_;
	VkDescriptorSetLayout global_descriptor_set_layout_;
	VkDescriptorSetLayout global_lights_descriptor_set_layout_;
	VkDescriptorSetLayout global_pbr_descriptor_set_layout_;

	std::shared_ptr<class LavaImage> default_texture_image_pink;
	std::shared_ptr<class LavaImage> default_texture_image_white;
	std::shared_ptr<class LavaImage> default_texture_image_black;
	std::vector<GlobalSceneData> global_scene_data_vector_;
	std::vector<VkDescriptorSet> global_descriptor_set_vector_;
	std::vector<std::unique_ptr<class LavaBuffer>> global_data_buffer_vector_;
	
	void initGlobalData();
	virtual bool shouldClose() override;
	virtual void beginFrame() override;
	virtual void endFrame() override;
	virtual void clearWindow() override;
	void clearWindow(uint32_t i);
	void clearColor(uint32_t i, float r, float g, float b, float a);
	virtual void pollEvents() override;
	virtual void updateMainCamera() override;
	void updateGlobalData(uint32_t view_index);
	void setDynamicViewportAndScissor(const VkExtent2D& extend);

	void prepareView(uint32_t i);
	void releaseView(uint32_t i);

	void immediate_submit(std::function<void(VkCommandBuffer)>&& function);

	LavaSessionVR& get_session() {
		return *session_.get();
	}

	bool is_session_running() const {
		return session_running_;
	}

	bool is_session_active() const {
		return session_active_;
	}

	uint32_t get_view_count() const{
		return view_count_;
	}

	LavaSwapchainVR& get_swapchain() {
		return *swapchain_.get();
	}

	glm::mat4 convertXrToGlm(const struct XrMatrix4x4f* xrMat);


	uint32_t color_image_index_ = 0;
	uint32_t depth_image_index_ = 0;
	VkCommandBuffer command_buffer_;
	std::vector<XrView> views_;
private:
	bool session_active_;
	RenderLayerInfo render_layer_info_;
	XrFrameState frame_state_{ XR_TYPE_FRAME_STATE };
	bool session_running_ = false;
	bool application_running_ = false;
	bool rendered_;

	uint32_t view_count_ = 0;
};



#endif // !__LAVA_ENGINE_VR_H__
