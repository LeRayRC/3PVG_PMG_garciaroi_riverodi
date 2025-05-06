#include "lava/vr/lava_engine_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_device.hpp"
#include "vr/lava_session_vr.hpp"
#include "vr/lava_swapchain_vr.hpp"
#include "vr/lava_blend_space_vr.hpp"
#include "lava/openxr_common/DebugOutput.h"
#include <openxr/openxr.h>


const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

LavaEngineVR::LavaEngineVR(XrPosef reference_pose) {
  instance_vr_ = std::make_unique<LavaInstanceVR>();

  binding_ = std::make_unique<LavaBindingVR>(*instance_vr_.get());

  instance_vulkan_ = std::make_unique<LavaInstance>(validationLayers,
    *instance_vr_.get(), *binding_.get());
    
  device_ = std::make_unique<LavaDevice>(*instance_vulkan_.get(),
    *instance_vr_.get(), *binding_.get());

  session_ = std::make_unique<LavaSessionVR>(*instance_vr_.get(),
    *instance_vulkan_.get(), *device_.get(), XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO);
  
  swapchain_ = std::make_unique<LavaSwapchainVR>(*instance_vr_.get(),
    *session_.get(), *device_.get());

  blend_space_ = std::make_unique<LavaBlendSpaceVR>(*instance_vr_.get(),
    *session_.get(), XR_ENVIRONMENT_BLEND_MODE_OPAQUE, reference_pose);
  
  application_running_ = true;
}

LavaEngineVR::~LavaEngineVR() {

}

bool LavaEngineVR::shouldClose() {
  return !application_running_;
}

void LavaEngineVR::beginFrame() {
  XrFrameWaitInfo frame_wait_info{ XR_TYPE_FRAME_WAIT_INFO };
  OPENXR_CHECK_INSTANCE(
    xrWaitFrame(session_->get_session(), &frame_wait_info, &frame_state_),
    "Failed to wait for XR Frame.",
    instance_vr_->get_instance());

  // Tell the OpenXR compositor that the application is beginning the frame.
  XrFrameBeginInfo frame_begin_info{ XR_TYPE_FRAME_BEGIN_INFO };
  OPENXR_CHECK_INSTANCE(
    xrBeginFrame(session_->get_session(), &frame_begin_info),
    "Failed to begin the XR Frame.",
    instance_vr_->get_instance());


  //Begin vulkan rendering
  XrSessionState session_state = session_->get_state();
  session_active_ = (session_state == XR_SESSION_STATE_SYNCHRONIZED || session_state == XR_SESSION_STATE_VISIBLE || session_state == XR_SESSION_STATE_FOCUSED);

  if (session_active_ && frame_state_.shouldRender) {
    //Acquire Vulkan Images


  }
}


void LavaEngineVR::endFrame() {
  std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = swapchain_->get_color_swapchain_infos();
  std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = swapchain_->get_depth_swapchain_infos();
  for (uint32_t i = 0; i < view_count_; i++) {
    LavaSwapchainVR::SwapchainInfo& color_swapchain_info = color_swapchain_info_vector[i];
    LavaSwapchainVR::SwapchainInfo& depth_swapchain_info = depth_swapchain_info_vector[i];

    // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
    XrSwapchainImageReleaseInfo release_info{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
    OPENXR_CHECK_INSTANCE(
      xrReleaseSwapchainImage(color_swapchain_info.swapchain, &release_info),
      "Failed to release Image back to the Color Swapchain",
      instance_vr_->get_instance());
    OPENXR_CHECK_INSTANCE(
      xrReleaseSwapchainImage(depth_swapchain_info.swapchain, &release_info),
      "Failed to release Image back to the Depth Swapchain",
      instance_vr_->get_instance());
  }

  render_layer_info_.layer_projection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
  render_layer_info_.layer_projection.space = blend_space_->get_space();
  render_layer_info_.layer_projection.viewCount = static_cast<uint32_t>(render_layer_info_.layer_projection_views.size());
  render_layer_info_.layer_projection.views = render_layer_info_.layer_projection_views.data();



  // Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
  XrFrameEndInfo frame_end_info{ XR_TYPE_FRAME_END_INFO };
  frame_end_info.displayTime = frame_state_.predictedDisplayTime;
  frame_end_info.environmentBlendMode = blend_space_->get_blend_mode();
  frame_end_info.layerCount = static_cast<uint32_t>(render_layer_info_.layers.size());
  frame_end_info.layers = render_layer_info_.layers.data();
  OPENXR_CHECK_INSTANCE(
    xrEndFrame(session_->get_session(), &frame_end_info),
    "Failed to end the XR Frame.",
    instance_vr_->get_instance());
}

bool LavaEngineVR::renderLayer() {
  
  // Locate the views from the view configuration within the (reference) space at the display time.
  std::vector<XrView> views(session_->get_view_configuration_views().size(), { XR_TYPE_VIEW });

  XrViewState view_state{ XR_TYPE_VIEW_STATE };  // Will contain information on whether the position and/or orientation is valid and/or tracked.
  XrViewLocateInfo view_locate_info{ XR_TYPE_VIEW_LOCATE_INFO };
  view_locate_info.viewConfigurationType = session_->get_configuration_view_type();
  view_locate_info.displayTime = render_layer_info_.predicted_display_time;
  view_locate_info.space = blend_space_->get_space();

  XrResult result = xrLocateViews(session_->get_session(), &view_locate_info, &view_state, static_cast<uint32_t>(views.size()), &view_count_, views.data());
  if (result != XR_SUCCESS) {
    XR_TUT_LOG("Failed to locate Views.");
    return false;
  }

  // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
  render_layer_info_.layer_projection_views.resize(view_count_, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });

  std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = swapchain_->get_color_swapchain_infos();
  std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = swapchain_->get_depth_swapchain_infos();
  // Per view in the view configuration:
  for (uint32_t i = 0; i < view_count_; i++) {
    LavaSwapchainVR::SwapchainInfo& color_swapchain_info = color_swapchain_info_vector[i];
    LavaSwapchainVR::SwapchainInfo& depth_swapchain_info = depth_swapchain_info_vector[i];

    // Acquire and wait for an image from the swapchains.
    // Get the image index of an image in the swapchains.
    // The timeout is infinite.

    XrSwapchainImageAcquireInfo acquire_info{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
    OPENXR_CHECK_INSTANCE(
      xrAcquireSwapchainImage(color_swapchain_info.swapchain, &acquire_info, &color_image_index_),
      "Failed to acquire Image from the Color Swapchian",
      instance_vr_->get_instance());
    OPENXR_CHECK_INSTANCE(
      xrAcquireSwapchainImage(depth_swapchain_info.swapchain, &acquire_info, &depth_image_index_),
      "Failed to acquire Image from the Depth Swapchian",
      instance_vr_->get_instance());

    XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
    wait_info.timeout = XR_INFINITE_DURATION;

    OPENXR_CHECK_INSTANCE(
      xrWaitSwapchainImage(color_swapchain_info.swapchain, &wait_info),
      "Failed to wait for Image from the Color Swapchain",
      instance_vr_->get_instance());

    OPENXR_CHECK_INSTANCE(
      xrWaitSwapchainImage(depth_swapchain_info.swapchain, &wait_info),
      "Failed to wait for Image from the Depth Swapchain",
      instance_vr_->get_instance());

    // Get the width and height and construct the viewport and scissors.
    
    const uint32_t& width = session_->get_view_configuration_views()[i].recommendedImageRectWidth;
    const uint32_t& height = session_->get_view_configuration_views()[i].recommendedImageRectHeight;
    GraphicsAPI::Viewport viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    GraphicsAPI::Rect2D scissor = { {(int32_t)0, (int32_t)0}, {width, height} };
    //MAYBE IT IS NECCESARRY TO INVERT DEPTH TEST
    float nearZ = 0.05f;
    float farZ = 100.0f;

    // Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
    // This also associates the swapchain image with this layer projection view.
    render_layer_info_.layer_projection_views[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
    render_layer_info_.layer_projection_views[i].pose = views[i].pose;
    render_layer_info_.layer_projection_views[i].fov = views[i].fov;
    render_layer_info_.layer_projection_views[i].subImage.swapchain = color_swapchain_info.swapchain;
    render_layer_info_.layer_projection_views[i].subImage.imageRect.offset.x = 0;
    render_layer_info_.layer_projection_views[i].subImage.imageRect.offset.y = 0;
    render_layer_info_.layer_projection_views[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
    render_layer_info_.layer_projection_views[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
    render_layer_info_.layer_projection_views[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering.

    // Rendering code to clear the color and depth image views.
    
    //m_graphicsAPI->BeginRendering();

    //if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE) {
    //  // VR mode use a background color.
    //  m_graphicsAPI->ClearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.17f, 0.17f, 0.17f, 1.00f);
    //}
    //else {
    //  // In AR mode make the background color black.
    //  m_graphicsAPI->ClearColor(colorSwapchainInfo.imageViews[colorImageIndex], 0.00f, 0.00f, 0.00f, 1.00f);
    //}
    //m_graphicsAPI->ClearDepth(depthSwapchainInfo.imageViews[depthImageIndex], 1.0f);
    //m_graphicsAPI->EndRendering();


  }

  // Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().

  return true;
}

void LavaEngineVR::clearWindow() {

}


void LavaEngineVR::pollEvents() {
  // Poll OpenXR for a new event.
  XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
  auto XrPollEvents = [&]() -> bool {
    eventData = { XR_TYPE_EVENT_DATA_BUFFER };
    return xrPollEvent(instance_vr_->get_instance(), &eventData) == XR_SUCCESS;
    };

  while (XrPollEvents()) {
    switch (eventData.type) {
      // Log the number of lost events from the runtime.
    case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
      XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
      XR_TUT_LOG("OPENXR: Events Lost: " << eventsLost->lostEventCount);
      break;
    }

    // Log that an instance loss is pending and shutdown the application.
    case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
      XrEventDataInstanceLossPending* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
      XR_TUT_LOG("OPENXR: Instance Loss Pending at: " << instanceLossPending->lossTime);
      session_running_ = false;
      application_running_ = false;
      break;
    }

    // Log that the interaction profile has changed.
    case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
      XrEventDataInteractionProfileChanged* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
      XR_TUT_LOG("OPENXR: Interaction Profile changed for Session: " << interactionProfileChanged->session);
      if (interactionProfileChanged->session != session_->get_session()) {
        XR_TUT_LOG("XrEventDataInteractionProfileChanged for unknown Session");
        break;
      }
      break;
    }

    // Log that there's a reference space change pending.
    case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
      XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
      XR_TUT_LOG("OPENXR: Reference Space Change pending for Session: " << referenceSpaceChangePending->session);
      if (referenceSpaceChangePending->session != session_->get_session()) {
        XR_TUT_LOG("XrEventDataReferenceSpaceChangePending for unknown Session");
        break;
      }
      break;
    }
    
    // Session State changes:
    case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
      XrEventDataSessionStateChanged* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
      if (sessionStateChanged->session != session_->get_session()) {
        XR_TUT_LOG("XrEventDataSessionStateChanged for unknown Session");
        break;
      }

      if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
        // SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
        XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        sessionBeginInfo.primaryViewConfigurationType = session_->get_configuration_view_type();
        OPENXR_CHECK_INSTANCE(xrBeginSession(session_->get_session(), &sessionBeginInfo), "Failed to begin Session.", instance_vr_->get_instance());
        session_running_ = true;
      }
      if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
        // SessionState is stopping. End the XrSession.
        OPENXR_CHECK_INSTANCE(xrEndSession(session_->get_session()), "Failed to end Session.", instance_vr_->get_instance());
        session_running_ = false;
      }
      if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
        // SessionState is exiting. Exit the application.
        session_running_ = false;
        application_running_ = false;
      }
      if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
        // SessionState is loss pending. Exit the application.
        // It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
        session_running_ = false;
        application_running_ = false;
      }
      // Store state for reference across the application.
      session_->set_state(sessionStateChanged->state);
      break;
    }
    default: {
      break;
    }
    }
  }

}
void LavaEngineVR::updateMainCamera() {

}
