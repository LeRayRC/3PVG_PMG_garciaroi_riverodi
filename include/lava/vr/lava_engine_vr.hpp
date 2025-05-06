#ifndef __LAVA_ENGINE_VR_H__
#define __LAVA_ENGINE_VR_H__ 1

//#ifndef XR_USE_GRAPHICS_API_VULKAN
//#define XR_USE_GRAPHICS_API_VULKAN 1
//#endif

#include "lava/common/lava_engine_base.hpp"
#include "lava/vr/lava_data_structures_vr.hpp"

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

	virtual bool shouldClose() override;
	virtual void beginFrame() override;
	virtual void endFrame() override;
	virtual void clearWindow() override;
	virtual void pollEvents() override;
	virtual void updateMainCamera() override;

	bool renderLayer();


private:
	bool session_active_;
	RenderLayerInfo render_layer_info_;
	XrFrameState frame_state_{ XR_TYPE_FRAME_STATE };
	bool session_running_ = false;
	bool application_running_ = false;

	uint32_t color_image_index_ = 0;
	uint32_t depth_image_index_ = 0;
	uint32_t view_count_ = 0;
};



#endif // !__LAVA_ENGINE_VR_H__
