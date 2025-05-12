#include "lava/vr/lava_engine_vr.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_device.hpp"
#include "vr/lava_session_vr.hpp"
#include "vr/lava_swapchain_vr.hpp"
#include "vr/lava_blend_space_vr.hpp"
#include "engine/lava_allocator.hpp"
#include "engine/lava_frame_data.hpp"
#include "lava/openxr_common/DebugOutput.h"
#include "engine/lava_vulkan_helpers.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "engine/lava_inmediate_communication.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_image.hpp"
#include "vr/xr_linear_algebra.hpp"
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
  
  inmediate_communication = std::make_unique<LavaInmediateCommunication>(*device_);

  allocator_ = std::make_unique<LavaAllocator>(*device_, *instance_vulkan_);
  for (int i = 0; i < FRAME_OVERLAP; i++) {
    frame_data_[i] = std::make_unique<LavaFrameData>(*device_, *allocator_);
  }
  
  global_descriptor_allocator_ = std::make_unique<LavaDescriptorManager>(device_->get_device(), LavaDescriptorManager::initial_sets, LavaDescriptorManager::pool_ratios);


  //Default data
  uint32_t pink_color_ = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
  default_texture_image_pink = std::make_shared<LavaImage>(this, (void*)&pink_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

  uint32_t white_color_ = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
  default_texture_image_white = std::make_shared<LavaImage>(this, (void*)&white_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

  uint32_t black_color_ = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
  default_texture_image_white = std::make_shared<LavaImage>(this, (void*)&black_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);



  initGlobalData();
  application_running_ = true;
}

LavaEngineVR::~LavaEngineVR() {

}

bool LavaEngineVR::shouldClose() {
  return !application_running_;
}

void LavaEngineVR::beginFrame() {
  chrono_now_ = std::chrono::steady_clock::now();
  rendered_ = false;
  render_layer_info_ = {};

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

  render_layer_info_.predicted_display_time = frame_state_.predictedDisplayTime;


  //Begin vulkan rendering
  XrSessionState session_state = session_->get_state();
  session_active_ = (session_state == XR_SESSION_STATE_SYNCHRONIZED || session_state == XR_SESSION_STATE_VISIBLE || session_state == XR_SESSION_STATE_FOCUSED);

  if (session_active_ && frame_state_.shouldRender) {
    views_.resize(session_->get_view_configuration_views().size(), { XR_TYPE_VIEW });
    XrViewState view_state{ XR_TYPE_VIEW_STATE };  // Will contain information on whether the position and/or orientation is valid and/or tracked.
    XrViewLocateInfo view_locate_info{ XR_TYPE_VIEW_LOCATE_INFO };
    view_locate_info.viewConfigurationType = session_->get_configuration_view_type();
    view_locate_info.displayTime = render_layer_info_.predicted_display_time;
    view_locate_info.space = blend_space_->get_space();

    XrResult result = xrLocateViews(session_->get_session(), &view_locate_info, &view_state, static_cast<uint32_t>(views_.size()), &view_count_, views_.data());
    if (result != XR_SUCCESS) {
      XR_TUT_LOG("Failed to locate Views.");
    }

    rendered_ = true;
  }
}


void LavaEngineVR::endFrame() {


  render_layer_info_.layer_projection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
  render_layer_info_.layer_projection.space = blend_space_->get_space();
  render_layer_info_.layer_projection.viewCount = static_cast<uint32_t>(render_layer_info_.layer_projection_views.size());
  render_layer_info_.layer_projection.views = render_layer_info_.layer_projection_views.data();

  if (rendered_) {
    render_layer_info_.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&render_layer_info_.layer_projection));
  }


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


  if (rendered_) {
    //increase the number of frames drawn
    for (int i = 0; i < FRAME_OVERLAP; i++) {
      frame_data_[i]->increaseFrameNumber();
    }
  }

  dt_ = std::chrono::duration_cast<std::chrono::microseconds>(chrono_now_ - chrono_last_update_).count() / 1000000.0f;
  chrono_last_update_ = chrono_now_;
}

void LavaEngineVR::prepareView(uint32_t i) {
  // Locate the views from the view configuration within the (reference) space at the display time.
  
  // Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
  render_layer_info_.layer_projection_views.resize(view_count_, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });
  
  std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = swapchain_->get_color_swapchain_infos();
  std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = swapchain_->get_depth_swapchain_infos();
  // Per view in the view configuration:
  //for (uint32_t i = 0; i < view_count_; i++) {
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
    render_layer_info_.layer_projection_views[i].pose = views_[i].pose;
    render_layer_info_.layer_projection_views[i].fov = views_[i].fov;
    render_layer_info_.layer_projection_views[i].subImage.swapchain = color_swapchain_info.swapchain;
    render_layer_info_.layer_projection_views[i].subImage.imageRect.offset.x = 0;
    render_layer_info_.layer_projection_views[i].subImage.imageRect.offset.y = 0;
    render_layer_info_.layer_projection_views[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
    render_layer_info_.layer_projection_views[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
    render_layer_info_.layer_projection_views[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering.

    //Begin Vulkan rendering
    VULKAN_CHECK(vkWaitForFences(device_->get_device(), 1,
      &frame_data_[i]->getCurrentFrame().render_fence,
      true, UINT64_MAX), "Failed to wait for fences");

    VULKAN_CHECK(vkResetFences(device_->get_device(), 1,
      &frame_data_[i]->getCurrentFrame().render_fence),
      "Failed to reset fences");

    command_buffer_ = frame_data_[i]->getCurrentFrame().main_command_buffer;
    VULKAN_CHECK(vkResetCommandBuffer(command_buffer_, 0),
      "Failed to reset command buffer");

    //Ahora se rellena la estructura del begin command buffer
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VULKAN_CHECK(vkBeginCommandBuffer(command_buffer_, &commandBufferBeginInfo),
      "Failed to begin command buffer");
}

void LavaEngineVR::releaseView(uint32_t i) {
  //Release Vulkan command buffer

  VULKAN_CHECK(vkEndCommandBuffer(command_buffer_),
    "Failed to end command buffer");

  VkSubmitInfo2 submit;
  VkCommandBufferSubmitInfo commandSubmitInfo = vkinit::CommandBufferSubmitInfo(command_buffer_);

  VkSemaphoreSubmitInfo signalInfo = vkinit::SemaphoreSubmitInfo(
    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame_data_[i]->getCurrentFrame().render_semaphore);

  if (&frame_data_[i]->getCurrentFrame() != &frame_data_[i]->getPreviousFrame()) {
    VkSemaphoreSubmitInfo waitInfo = vkinit::SemaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame_data_[i]->getPreviousFrame().render_semaphore);
     submit = vkinit::SubmitInfo(&commandSubmitInfo, &signalInfo, &waitInfo);
  }
  else {
     submit = vkinit::SubmitInfo(&commandSubmitInfo, &signalInfo, nullptr);
  }
  
  //VkSemaphoreSubmitInfo waitInfo = vkinit::SemaphoreSubmitInfo(
  //  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame_data_[i]->getCurrentFrame().swap_chain_semaphore);
  

  

  VULKAN_CHECK(vkQueueSubmit2(device_->get_graphics_queue(), 1,
    &submit, frame_data_[i]->getCurrentFrame().render_fence),
    "Failed to submit queue2");

  std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = swapchain_->get_color_swapchain_infos();
  std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = swapchain_->get_depth_swapchain_infos();
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

void LavaEngineVR::clearWindow() {}


void LavaEngineVR::clearWindow(uint32_t i) {
  //Convertimos la imagen de dibujado a escribible
  std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = swapchain_->get_color_swapchain_infos();
  std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = swapchain_->get_depth_swapchain_infos();
  // Per view in the view configuration:
  //for (uint32_t i = 0; i < view_count_; i++) {
  LavaSwapchainVR::SwapchainInfo& color_swapchain_info = color_swapchain_info_vector[i];
  LavaSwapchainVR::SwapchainInfo& depth_swapchain_info = depth_swapchain_info_vector[i];


  VkImage image = swapchain_->get_image_from_image_view(color_swapchain_info.imageViews[color_image_index_]);
  TransitionImage(command_buffer_, image,
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  //Limpiamos la imagen con un clear color
  VkClearColorValue clearValue;
  clearValue = { {0.0f,0.0f,0.0f,0.0f} };

  //Seleccionamos un rango de la imagen sobre la que actuar
  VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

  //Aplicamos el clear color a una imagen 
  vkCmdClearColorImage(command_buffer_, image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);



}

void LavaEngineVR::clearColor(uint32_t i, float r, float g, float b, float a) {
  //Convertimos la imagen de dibujado a escribible
  std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = swapchain_->get_color_swapchain_infos();
  std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = swapchain_->get_depth_swapchain_infos();
  // Per view in the view configuration:
  //for (uint32_t i = 0; i < view_count_; i++) {
  LavaSwapchainVR::SwapchainInfo& color_swapchain_info = color_swapchain_info_vector[i];
  LavaSwapchainVR::SwapchainInfo& depth_swapchain_info = depth_swapchain_info_vector[i];

  VkClearColorValue clearColor;
  clearColor.float32[0] = r;
  clearColor.float32[1] = g;
  clearColor.float32[2] = b;
  clearColor.float32[3] = a;

  VkImageSubresourceRange range;
  range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  range.baseMipLevel = 0;
  range.levelCount = 1;
  range.baseArrayLayer = 0;
  range.layerCount = 1;

  VkImage image = swapchain_->get_image_from_image_view(color_swapchain_info.imageViews[color_image_index_]);

  VkImageMemoryBarrier imageBarrier;
  imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageBarrier.pNext = nullptr;
  imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.image = image;
  imageBarrier.subresourceRange = range;
  vkCmdPipelineBarrier(command_buffer_, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlagBits(0), 0, nullptr, 0, nullptr, 1, &imageBarrier);

  vkCmdClearColorImage(command_buffer_, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

  imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.image = image;
  imageBarrier.subresourceRange = range;
  vkCmdPipelineBarrier(command_buffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkDependencyFlagBits(0), 0, nullptr, 0, nullptr, 1, &imageBarrier);
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

void LavaEngineVR::initGlobalData() {
  int amount = session_->get_view_configuration_views().size();
  global_scene_data_vector_.resize(amount);
  global_descriptor_set_vector_.resize(amount);
  global_data_buffer_vector_.resize(amount);

  global_descriptor_allocator_->clear();
  DescriptorLayoutBuilder builder;
  builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  global_descriptor_set_layout_ = builder.build(device_->get_device(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

  for (int i = 0; i < amount; i++) {
    global_descriptor_set_vector_[i] = global_descriptor_allocator_->allocate(global_descriptor_set_layout_);
    global_data_buffer_vector_[i] = std::make_unique<LavaBuffer>(*allocator_, sizeof(GlobalSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    global_data_buffer_vector_[i]->setMappedData();
    global_descriptor_allocator_->clear();

    global_descriptor_allocator_->writeBuffer(0, global_data_buffer_vector_[i]->buffer_.buffer, sizeof(GlobalSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    global_descriptor_allocator_->updateSet(global_descriptor_set_vector_[i]);
    global_descriptor_allocator_->clear();
  }

  //Descriptor set layout of every light
  builder.clear();
  builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  builder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  builder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  builder.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  builder.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  global_lights_descriptor_set_layout_ = builder.build(device_->get_device(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);

  builder.clear();
  builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // base color texture
  builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // normal
  builder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // roughness_metallic_texture
  builder.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // opacity
  builder.addBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  builder.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // opacity
  global_pbr_descriptor_set_layout_ = builder.build(device_->get_device(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void LavaEngineVR::updateGlobalData(uint32_t view_index) {
  //Detect input
  XrMatrix4x4f viewProj;
  XrMatrix4x4f proj;
  XrMatrix4x4f_CreateProjectionFov(&proj, VULKAN, views_[view_index].fov, 0.05f , 50.0f);
  XrMatrix4x4f toView;
  XrVector3f scale1m{ 1.0f, 1.0f, 1.0f };
  XrMatrix4x4f_CreateTranslationRotationScale(&toView, &views_[view_index].pose.position, &views_[view_index].pose.orientation, &scale1m);
  XrMatrix4x4f view;
  XrMatrix4x4f_InvertRigidBody(&view, &toView);
  XrMatrix4x4f_Multiply(&viewProj, &proj, &view);
  
  global_scene_data_vector_[view_index].viewproj = convertXrToGlm(&viewProj);
  global_scene_data_vector_[view_index].proj = convertXrToGlm(&proj);
  global_scene_data_vector_[view_index].view = convertXrToGlm(&view);
  XrVector3f vector_pos = views_[view_index].pose.position;
  global_scene_data_vector_[view_index].cameraPos = glm::vec3(vector_pos.x, vector_pos.y, vector_pos.z);

  global_data_buffer_vector_[view_index]->updateBufferData(&global_scene_data_vector_[view_index], sizeof(GlobalSceneData));

}

void LavaEngineVR::setDynamicViewportAndScissor(const VkExtent2D& extend) {
  //set dynamic viewport and scissor
  VkViewport viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = (float)extend.width;
  viewport.height = (float)extend.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer_, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = (uint32_t)extend.width;//swap_chain_.get_draw_extent().width;
  scissor.extent.height = (uint32_t)extend.height;//swap_chain_.get_draw_extent().height;
  vkCmdSetScissor(command_buffer_, 0, 1, &scissor);
}



void LavaEngineVR::immediate_submit(std::function<void(VkCommandBuffer)>&& function) {
  VkFence aux_inmediate_fence = inmediate_communication->get_inmediate_fence();
  if (vkResetFences(device_->get_device(), 1, &aux_inmediate_fence) != VK_SUCCESS) {
    exit(-1);
  }

  VkCommandBuffer aux_immediate_command_buffer = inmediate_communication->get_immediate_command_buffer();
  if (vkResetCommandBuffer(aux_immediate_command_buffer, 0) != VK_SUCCESS) {
    exit(-1);
  }

  VkCommandBuffer command_buffer = aux_immediate_command_buffer;
  VkCommandBufferBeginInfo command_buffer_begin_info =
    vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

  function(command_buffer);

  vkEndCommandBuffer(command_buffer);

  VkCommandBufferSubmitInfo command_submit_info = vkinit::CommandBufferSubmitInfo(command_buffer);
  VkSubmitInfo2 submit = vkinit::SubmitInfo(&command_submit_info, nullptr, nullptr);

  vkQueueSubmit2(device_->get_graphics_queue(), 1, &submit, aux_inmediate_fence);


  vkWaitForFences(device_->get_device(), 1, &aux_inmediate_fence, true, 9999999999);
}

glm::mat4 LavaEngineVR::convertXrToGlm(const XrMatrix4x4f* xrMat) {
  return glm::mat4(
    xrMat->m[0], xrMat->m[1],   xrMat->m[2],  xrMat->m[3],
    xrMat->m[4], xrMat->m[5],   xrMat->m[6],  xrMat->m[7],
    xrMat->m[8], xrMat->m[9],   xrMat->m[10], xrMat->m[11],
    xrMat->m[12], xrMat->m[13], xrMat->m[14], xrMat->m[15]
  );
}