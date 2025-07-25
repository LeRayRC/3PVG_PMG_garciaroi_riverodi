/**
 * @file custom_engine.hpp
 * @author ???
 * @brief Custom Engine's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */
#include "lava/engine/lava_engine.hpp"
#include "lava_vulkan_helpers.hpp"
#include "lava_vulkan_inits.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/ecs/lava_ecs_components.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "lava/engine/lava_descriptors.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_pipeline_builder.hpp"
#include "lava/engine/lava_image.hpp"
#include "engine/lava_surface.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_swap_chain.hpp"
#include "engine/lava_allocator.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_inmediate_communication.hpp"
#include "engine/lava_pipeline.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "engine/lava_descriptor_manager.hpp"
#include <future>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"



#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

std::vector<std::function<void()>> LavaEngine::end_frame_callbacks;

LavaEngine::LavaEngine(unsigned int window_width, unsigned int window_height) :
	//Init base engine
	instance_{ std::make_unique<LavaInstance>(validationLayers) },
	surface_{ std::make_unique<LavaSurface>(instance_->get_instance(), window_.get_window()) },
	window_{ window_width, window_height, "LavaEngine" },
	device_{ std::make_unique<LavaDevice>(*instance_,*surface_) },
	window_extent_{ window_width, window_height },
	allocator_{ std::make_unique<LavaAllocator>(*device_, *instance_)},
	swap_chain_{ std::make_unique<LavaSwapChain>(*device_, *surface_, window_extent_, allocator_->get_allocator())},
	frame_data_{ std::make_unique<LavaFrameData>(*device_, *surface_, *allocator_, &global_scene_data_)},
	inmediate_communication{ std::make_unique<LavaInmediateCommunication>(*device_, *surface_)},
	global_descriptor_allocator_{ std::make_unique<LavaDescriptorManager>(device_->get_device(),LavaDescriptorManager::initial_sets,LavaDescriptorManager::pool_ratios)}
{
	stop_rendering = false;
	//GLOBAL DATA
	initGlobalData();

	//Build proj and view matrix
	global_scene_data_.proj = glm::perspective(glm::radians(90.0f),
		(float)swap_chain_->get_draw_extent().width / (float)swap_chain_->get_draw_extent().height, 10000.f, 0.1f);
	global_scene_data_.proj[1][1] *= -1;
	global_scene_data_.view = glm::mat4(1.0f);
	global_scene_data_.viewproj = global_scene_data_.proj * global_scene_data_.view;
	global_scene_data_.gbuffer_render_selected = GBUFFER_ALBEDO;
	global_data_buffer_->updateBufferData(&global_scene_data_, sizeof(GlobalSceneData));

	//Default data
	uint32_t pink_color_ = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	default_texture_image_pink = std::make_shared<LavaImage>(this, (void*)&pink_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	uint32_t white_color_ = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	default_texture_image_white = std::make_shared<LavaImage>(this, (void*)&white_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	uint32_t black_color_ = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
	default_texture_image_black = std::make_shared<LavaImage>(this, (void*)&black_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);


	initImgui();
	swap_chain_image_index = 0;
}

LavaEngine::~LavaEngine(){
	vkDeviceWaitIdle(device_->get_device());
	vkDestroyDescriptorSetLayout(device_->get_device(), global_descriptor_set_layout_, nullptr);
	vkDestroyDescriptorSetLayout(device_->get_device(), global_lights_descriptor_set_layout_, nullptr);
	vkDestroyDescriptorSetLayout(device_->get_device(), global_pbr_descriptor_set_layout_, nullptr);
	ImGui_ImplVulkan_Shutdown();
	imgui_descriptor_alloc.destroy_pool(device_->get_device());
}

GLFWwindow* LavaEngine::get_window() const {
	return window_.get_window();
}

VkSurfaceKHR LavaEngine::get_surface() const {
	return surface_->get_surface();
}


VkInstance LavaEngine::get_instance() {
	return instance_->get_instance();
}

void LavaEngine::initGlobalData() {
	global_descriptor_allocator_->clear();
	DescriptorLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	global_descriptor_set_layout_ = builder.build(device_->get_device(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	global_descriptor_set_ = global_descriptor_allocator_->allocate(global_descriptor_set_layout_);
	global_data_buffer_ = std::make_unique<LavaBuffer>(*allocator_, sizeof(GlobalSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	global_data_buffer_->setMappedData();
	global_descriptor_allocator_->clear();

	global_descriptor_allocator_->writeBuffer(0, global_data_buffer_->buffer_.buffer, sizeof(GlobalSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	global_descriptor_allocator_->updateSet(global_descriptor_set_);
	global_descriptor_allocator_->clear();

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

void LavaEngine::beginFrame() {
	chrono_now_ = std::chrono::steady_clock::now();

	updateMainCamera();
	//Check every light component to allocate or free again

	//Esperamos a que el fence comunique que la grafica ya ha terminado de dibujar
	//El timeout esta en nanosegundos 10e-9
	VkResult fenceStatus = vkWaitForFences(device_->get_device(), 1,
		&frame_data_->getPreviousFrame().render_fence,
		true, UINT64_MAX);



	if (fenceStatus == VK_TIMEOUT) {
		printf("Error: GPU no responde - timeout excedido!");
		// Aqu� deber�as manejar el error adecuadamente
		exit(-2);
	}
	else if (fenceStatus != VK_SUCCESS) {
		printf("Error inesperado al esperar el fence!");
		exit(-2);
	}

	//Reseteamos el fence
	if (vkResetFences(device_->get_device(), 1, &frame_data_->getCurrentFrame().render_fence) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence restart failed!");
#endif // !NDEBUG
	}


	glfwPollEvents();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	//Solicitamos una imagen del swap chain
	//uint32_t swap_chain_image_index;
	if (vkAcquireNextImageKHR(device_->get_device(), swap_chain_->get_swap_chain(), UINT64_MAX,
		frame_data_->getCurrentFrame().swap_chain_semaphore, nullptr, &swap_chain_image_index) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Swapchain image not retrieved!");
#endif // !NDEBUG
	}

	//Se resetea e inicia el command buffer del frame actual
	//VkCommandBuffer commandBuffer = frame_data_.getCurrentFrame().main_command_buffer;
	commandBuffer = frame_data_->getCurrentFrame().main_command_buffer;
	if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Resseting commandbuffer failed!");
#endif // !NDEBUG
	}

	frame_data_->getCurrentFrame().last_bound_mesh.reset();

	//Ahora se rellena la estructura del begin command buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Begin commandbuffer failed!");
#endif // !NDEBUG
	}
}

void LavaEngine::endFrame() {
	ImGui::Render();

	VkResult result;

	VkImageLayout oldLayout = (frame_data_->frame_number_ == 0) ?
		VK_IMAGE_LAYOUT_UNDEFINED :
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;


	// Cambiamos la imagen del swap chain para poder escribir sobre ella
	//TransitionImage(commandBuffer, swap_chain_->get_swap_chain_images()[swap_chain_image_index],
	//	oldLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	AdvancedTransitionImage(commandBuffer, swap_chain_->get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//draw imgui into the swapchain image
	drawImgui(commandBuffer, swap_chain_->get_swap_chain_image_views()[swap_chain_image_index]);

	AdvancedTransitionImage(commandBuffer, swap_chain_->get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//Se cambia la imagen al layout presentable
	//TransitionImage(commandBuffer, swap_chain_->get_swap_chain_images()[swap_chain_image_index],
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//Se finalizar el command buffer
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("End commandbuffer failed!");
#endif // !NDEBUG
	}

	VkCommandBufferSubmitInfo commandSubmitInfo = vkinit::CommandBufferSubmitInfo(commandBuffer);
	VkSemaphoreSubmitInfo waitInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame_data_->getCurrentFrame().swap_chain_semaphore);
	VkSemaphoreSubmitInfo signalInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame_data_->getCurrentFrame().render_semaphore);

	VkSubmitInfo2 submit = vkinit::SubmitInfo(&commandSubmitInfo, &signalInfo, &waitInfo);

	result = vkQueueSubmit2(device_->get_graphics_queue(), 1, &submit, frame_data_->getCurrentFrame().render_fence);
	if (result != VK_SUCCESS) {
		printf("Error at Queue submit!\n");
	}
	
	
	//Se crea la estructura de presentacion para enviarla a la ventana de GLFW
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	VkSwapchainKHR aux_swap = swap_chain_->get_swap_chain();
	presentInfo.pSwapchains = &aux_swap;
	presentInfo.swapchainCount = 1;
	
	presentInfo.pWaitSemaphores = &frame_data_->getCurrentFrame().render_semaphore;
	presentInfo.waitSemaphoreCount = 1;
	
	presentInfo.pImageIndices = &swap_chain_image_index;
	
	result = vkQueuePresentKHR(device_->get_present_queue(), &presentInfo);
	if (result != VK_SUCCESS) {
		printf("Error at Queue present!\n");
	}


	//increase the number of frames drawn
	frame_data_->increaseFrameNumber();

	//End Frame Callbacks(NEEDS A WRAPER)
	for (auto it = end_frame_callbacks.rbegin(); it != end_frame_callbacks.rend(); it++) {
		(*it)();
	}

	dt_ = std::chrono::duration_cast<std::chrono::microseconds>(chrono_now_ - chrono_last_update_).count() / 1000000.0f;
	chrono_last_update_ = chrono_now_;
	//vkDeviceWaitIdle(device_->get_device());
}

void LavaEngine::clearWindow() {
	//Convertimos la imagen de dibujado a escribible
	TransitionImage(commandBuffer, swap_chain_->get_draw_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	//Limpiamos la imagen con un clear color
	VkClearColorValue clearValue;
	clearValue = { {0.0f,0.0f,0.0f,1.0f} };

	//Seleccionamos un rango de la imagen sobre la que actuar
	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//Aplicamos el clear color a una imagen 
	vkCmdClearColorImage(commandBuffer, swap_chain_->get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}

void LavaEngine::initImgui() {
	
	std::vector<DescriptorAllocator::PoolSizeRatio> pool_sizes = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 }
	};

	imgui_descriptor_alloc.init_pool(device_->get_device(), 1000, pool_sizes);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan(get_window(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = get_instance();
	init_info.PhysicalDevice = device_->get_physical_device();
	init_info.Device = device_->get_device();
	init_info.Queue = device_->get_graphics_queue();
	init_info.DescriptorPool = imgui_descriptor_alloc.pool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.UseDynamicRendering = true;
	init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	VkFormat swap_format = swap_chain_->get_swap_chain_image_format();
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swap_format;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();
}

void LavaEngine::immediate_submit(std::function<void(VkCommandBuffer)>&& function) {
	VkFence aux_inmediate_fence = inmediate_communication->get_inmediate_fence();
	if (vkResetFences(device_->get_device(), 1, &aux_inmediate_fence) != VK_SUCCESS) {
		exit(-1);
	}

	VkCommandBuffer aux_immediate_command_buffer = inmediate_communication->get_immediate_command_buffer();
	if (vkResetCommandBuffer(aux_immediate_command_buffer,0) != VK_SUCCESS) {
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

	vkQueueSubmit2(device_->get_transfer_queue(), 1, &submit, aux_inmediate_fence);
	

	vkWaitForFences(device_->get_device(), 1, &aux_inmediate_fence, true, 9999999999);
}

void LavaEngine::drawImgui(VkCommandBuffer command_buffer, VkImageView target_image_view) {
	VkRenderingAttachmentInfo color_attachment =
		vkinit::AttachmentInfo(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo render_info = vkinit::RenderingInfo(window_extent_, &color_attachment, nullptr);

	vkCmdBeginRendering(command_buffer, &render_info);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

	vkCmdEndRendering(command_buffer);
}

void LavaEngine::renderImgui() {
	ImGui::ShowDemoWindow();
}

void LavaEngine::setDynamicViewportAndScissor(const VkExtent2D& extend) {
	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)extend.width;
	viewport.height = (float)extend.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = (uint32_t)extend.width;//swap_chain_.get_draw_extent().width;
	scissor.extent.height = (uint32_t)extend.height;//swap_chain_.get_draw_extent().height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LavaEngine::updateMainCamera() {

	//Detect input
	assert(main_camera_camera_ != nullptr);
	assert(main_camera_transform_ != nullptr);

	main_camera_camera_->view_ = GenerateViewMatrix(main_camera_transform_->pos_, main_camera_transform_->rot_);
	global_scene_data_.cameraPos = main_camera_transform_->pos_;
	global_scene_data_.view = main_camera_camera_->view_;

	if (main_camera_camera_->type_ == CameraType_Perspective) {
		global_scene_data_.proj = glm::perspective(glm::radians(main_camera_camera_->fov_),
		(float)window_extent_.width / (float)window_extent_.height, main_camera_camera_->near_, main_camera_camera_->far_);
	}
	else {
		float left = -main_camera_camera_->size_;
		float right = main_camera_camera_->size_;
		float bottom = -main_camera_camera_->size_;
		float top = main_camera_camera_->size_;
		global_scene_data_.proj = glm::ortho(left, right, bottom, top, main_camera_camera_->near_, main_camera_camera_->far_);
	}

	global_scene_data_.proj[1][1] *= -1;
	global_scene_data_.viewproj = global_scene_data_.proj * global_scene_data_.view;

	global_data_buffer_->updateBufferData(&global_scene_data_, sizeof(GlobalSceneData));
}


