#ifndef __LAVA_BLEND_SPACE_H__
#define __LAVA_BLEND_SPACE_H__ 1

#include <openxr/openxr.h>

class LavaBlendSpaceVR
{
public:
	LavaBlendSpaceVR(class LavaInstanceVR& instance_vr,
    class LavaSessionVR& session,
    XrEnvironmentBlendMode preferred_blend_mode,
    XrPosef reference_pose);
	~LavaBlendSpaceVR();

  XrEnvironmentBlendMode get_blend_mode() {
    return environment_blend_mode_;
  }

private:
  
  XrEnvironmentBlendMode environment_blend_mode_ = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
  XrSpace space_ = XR_NULL_HANDLE;

  LavaInstanceVR& instance_vr_;
};



#endif
