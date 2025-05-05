#ifndef __LAVA_ENGINE_VR_H__
#define __LAVA_ENGINE_VR_H__ 1

//#ifndef XR_USE_GRAPHICS_API_VULKAN
//#define XR_USE_GRAPHICS_API_VULKAN 1
//#endif

#include "lava/common/lava_engine_base.hpp"

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


private:
	bool session_running_ = false;
	bool application_running_ = false;
};



#endif // !__LAVA_ENGINE_VR_H__
