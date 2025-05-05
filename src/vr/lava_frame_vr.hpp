#ifndef __LAVA_FRAME_VR_H__
#define __LAVA_FRAME_VR_H__ 1

class LavaFrameVR
{
public:
	LavaFrameVR(class LavaInstanceVR& instance_vr, 
		class LavaSessionVR& session, class LavaBlendSpaceVR& blend_space);
	~LavaFrameVR();

	void beginFrame();
	void waitFrame();
	void endFrame();

private:
	class LavaSessionVR& session_;
	class LavaInstanceVR& instance_vr_;
	class LavaBlendSpaceVR& blend_space_;

	XrFrameState frame_state_{ XR_TYPE_FRAME_STATE };
};



#endif // !__LAVA_FRAME_H__
