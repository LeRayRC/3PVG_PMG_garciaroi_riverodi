#include "vr/lava_blend_space_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_session_vr.hpp"
#include "lava/openxr_common/DebugOutput.h"
#include "lava/openxr_common/OpenXRHelper.h"

LavaBlendSpaceVR::LavaBlendSpaceVR(LavaInstanceVR& instance_vr,
  LavaSessionVR& session,
  XrEnvironmentBlendMode preferred_blend_mode,
  XrPosef reference_pose) :
  instance_vr_{instance_vr}
{
  std::vector<XrEnvironmentBlendMode> environment_blend_modes = {};
  // Retrieves the available blend modes. The first call gets the count of the array that will be returned. The next call fills out the array.
  uint32_t environment_blend_mode_count = 0;
  OPENXR_CHECK_INSTANCE(
    xrEnumerateEnvironmentBlendModes(instance_vr_.get_instance(), instance_vr_.get_system_id(), session.get_configuration_view_type(), 0, &environment_blend_mode_count, nullptr),
    "Failed to enumerate EnvironmentBlend Modes.",
    instance_vr_.get_instance());
  environment_blend_modes.resize(environment_blend_mode_count);
  OPENXR_CHECK_INSTANCE(xrEnumerateEnvironmentBlendModes(instance_vr_.get_instance(), instance_vr_.get_system_id(), session.get_configuration_view_type(), environment_blend_mode_count, &environment_blend_mode_count, environment_blend_modes.data()),
    "Failed to enumerate EnvironmentBlend Modes.",
    instance_vr_.get_instance());

  // Pick the first application supported blend mode supported by the hardware.
  if (std::find(environment_blend_modes.begin(), environment_blend_modes.end(), preferred_blend_mode) != environment_blend_modes.end()) {
    environment_blend_mode_ = preferred_blend_mode;
  }
  
  if (environment_blend_mode_ == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM) {
    XR_TUT_LOG_ERROR("Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE.");
    environment_blend_mode_ = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
  }

  // Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace,
  // specifying a Local space with an identity pose as the origin.
  XrReferenceSpaceCreateInfo referenceSpaceCI{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
  referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
  //IMPORTANT TO CHECK!!
  referenceSpaceCI.poseInReferenceSpace = reference_pose;  
  OPENXR_CHECK_INSTANCE(
    xrCreateReferenceSpace(session.get_session(), &referenceSpaceCI, &space_),
    "Failed to create ReferenceSpace.",
    instance_vr_.get_instance());
}

LavaBlendSpaceVR::~LavaBlendSpaceVR()
{
  OPENXR_CHECK_INSTANCE(xrDestroySpace(space_), "Failed to destroy Space.", instance_vr_.get_instance());
}