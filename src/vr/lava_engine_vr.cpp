#include "lava/vr/lava_engine_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_device.hpp"
#include "vr/lava_session_vr.hpp"
#include "lava/openxr_common/DebugOutput.h"


const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

LavaEngineVR::LavaEngineVR() {
  instance_vr_ = std::make_unique<LavaInstanceVR>();

  binding_ = std::make_unique<LavaBindingVR>(*instance_vr_.get());

  instance_vulkan_ = std::make_unique<LavaInstance>(validationLayers,
    *instance_vr_.get(), *binding_.get());
    
  device_ = std::make_unique<LavaDevice>(*instance_vulkan_.get(),
    *instance_vr_.get(), *binding_.get());

  session_ = std::make_unique<LavaSessionVR>(*instance_vr_.get(),
    *instance_vulkan_.get(), *device_.get(), XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO);
  
  application_running_ = true;
}

LavaEngineVR::~LavaEngineVR() {

}

bool LavaEngineVR::shouldClose() {
  return !application_running_;
}

void LavaEngineVR::beginFrame() {

}
void LavaEngineVR::endFrame() {

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
