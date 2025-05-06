#include "vr/lava_frame_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_session_vr.hpp"
#include "vr/lava_blend_space_vr.hpp"
#include "lava/vr/lava_data_structures_vr.hpp"

LavaFrameVR::LavaFrameVR(LavaInstanceVR& instance_vr,
	LavaSessionVR& session,LavaBlendSpaceVR& blend_space) :
	instance_vr_{instance_vr},
	session_{session},
	blend_space_{blend_space}
{


}

void LavaFrameVR::beginFrame() {
	XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	OPENXR_CHECK_INSTANCE(
		xrWaitFrame(session_.get_session(), &frameWaitInfo, &frame_state_),
		"Failed to wait for XR Frame.",
		instance_vr_.get_instance());

	// Tell the OpenXR compositor that the application is beginning the frame.
	XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	OPENXR_CHECK_INSTANCE(
		xrBeginFrame(session_.get_session(), &frameBeginInfo),
		"Failed to begin the XR Frame.",
		instance_vr_.get_instance());
}

void LavaFrameVR::endFrame(RenderLayerInfo& render_layer_info) {
	// Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = frame_state_.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = blend_space_.get_blend_mode();
	frameEndInfo.layerCount = static_cast<uint32_t>(render_layer_info.layers.size());
	frameEndInfo.layers = render_layer_info.layers.data();
	OPENXR_CHECK_INSTANCE(
		xrEndFrame(session_.get_session(), &frameEndInfo),
		"Failed to end the XR Frame.",
		instance_vr_.get_instance());
}




LavaFrameVR::~LavaFrameVR()
{
}