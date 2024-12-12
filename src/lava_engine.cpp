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
#include "lava_engine.hpp"
#include "lava_vulkan_helpers.hpp"
#include "lava_vulkan_inits.hpp"
#include "engine/lava_pipeline_builder.hpp"
#include "engine/lava_image.hpp"
#include "lava_transform.hpp"
#include <chrono>


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

LavaEngine* loaded_engine = nullptr;
std::vector<std::function<void()>> LavaEngine::end_frame_callbacks;

LavaEngine::LavaEngine() :
	global_scene_data_{ glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f),glm::vec4(0.0f)},
	surface_{ instance_.get_instance(), window_.get_window() },
	instance_{ validationLayers },
	window_{ 1280, 720, "LavaEngine" },
	device_{ instance_,surface_ },
	window_extent_{ 1280, 720 },
	allocator_{device_, instance_},
	swap_chain_{ device_, surface_, window_extent_, allocator_.get_allocator()},
	frame_data_{device_, surface_, allocator_, &global_scene_data_},
	inmediate_communication{device_, surface_},
	global_descriptor_allocator_{device_.get_device(),LavaDescriptorManager::initial_sets,LavaDescriptorManager::pool_ratios},
	dt_{0.0f}
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;
	is_initialized_ = false;
	stop_rendering = false;
	//GLOBAL DATA
	initGlobalData();


	//Build proj and view matrix
	camera_parameters_.fov = 90.0f;
	global_scene_data_.proj = glm::perspective(glm::radians(camera_parameters_.fov),
		(float)swap_chain_.get_draw_extent().width / (float)swap_chain_.get_draw_extent().height, 10000.f, 0.1f);
	global_scene_data_.proj[1][1] *= -1;
	global_scene_data_.view = glm::mat4(1.0f);
	global_scene_data_.viewproj = global_scene_data_.proj * global_scene_data_.view;
	global_data_buffer_->updateBufferData(&global_scene_data_, sizeof(GlobalSceneData));


	//Default data
	pink_color_ = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	default_texture_image_ = std::make_shared<LavaImage>(this, (void*)&pink_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	initImgui();
	is_initialized_ = true;
	swap_chain_image_index = 0;
}

LavaEngine::LavaEngine(unsigned int window_width, unsigned int window_height) :
	global_scene_data_{ glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f),glm::vec4(1.0f) },
	window_{window_width, window_height, "LavaEngine"},
	instance_{validationLayers},
	surface_{instance_.get_instance(), window_.get_window()},
	device_{ instance_,surface_ },
	window_extent_{window_width, window_height},
	allocator_{ device_, instance_ },
	swap_chain_{ device_, surface_, window_extent_, allocator_.get_allocator() },
	frame_data_{ device_, surface_, allocator_, &global_scene_data_ },
	inmediate_communication{ device_, surface_ },
	global_descriptor_allocator_{ device_.get_device(),LavaDescriptorManager::initial_sets,LavaDescriptorManager::pool_ratios },
	dt_{ 0.0f }
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;
	is_initialized_ = false;
	stop_rendering = false;
	//GLOBAL DATA
	initGlobalData();

	camera_parameters_.fov = 90.0f;
	global_scene_data_.proj = glm::perspective(glm::radians(camera_parameters_.fov),
		(float)swap_chain_.get_draw_extent().width / (float)swap_chain_.get_draw_extent().height, 10000.f, 0.1f);
	global_scene_data_.proj[1][1] *= -1;
	global_scene_data_.view = glm::mat4(1.0f);
	global_scene_data_.viewproj = global_scene_data_.proj * global_scene_data_.view;
	global_data_buffer_->updateBufferData(&global_scene_data_, sizeof(GlobalSceneData));

	//Default data
	pink_color_ = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	default_texture_image_ = std::make_shared<LavaImage>(this, (void*)&pink_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	initImgui();
	is_initialized_ = true;
	swap_chain_image_index = 0;
}

LavaEngine::~LavaEngine(){
	vkDeviceWaitIdle(device_.get_device());
	vkDestroyDescriptorSetLayout(device_.get_device(), global_descriptor_set_layout_, nullptr);
	//pipelines_.clear();
	/*main_deletion_queue_.flush();*/
	ImGui_ImplVulkan_Shutdown();
	imgui_descriptor_alloc.destroy_pool(device_.get_device());
}

VkInstance LavaEngine::get_instance() const{
	return instance_.get_instance();
}

GLFWwindow* LavaEngine::get_window() const {
	return window_.get_window();
}

VkSurfaceKHR LavaEngine::get_surface() const {
	return surface_.get_surface();
}

void LavaEngine::initGlobalData() {
	global_descriptor_allocator_.clear();
	DescriptorLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	global_descriptor_set_layout_ = builder.build(device_.get_device(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	global_descriptor_set_ = global_descriptor_allocator_.allocate(global_descriptor_set_layout_);
	global_data_buffer_ = std::make_unique<LavaBuffer>(allocator_, sizeof(GlobalSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	global_data_buffer_->setMappedData();
	global_descriptor_allocator_.writeBuffer(0, global_data_buffer_->buffer_.buffer, sizeof(GlobalSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	global_descriptor_allocator_.updateSet(global_descriptor_set_);
	global_descriptor_allocator_.clear();
}

void LavaEngine::beginFrame() {
	chrono_now_ = std::chrono::steady_clock::now();


	glfwPollEvents();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	//Esperamos a que el fence comunique que la grafica ya ha terminado de dibujar
//El timeout esta en nanosegundos 10e-9
	if (vkWaitForFences(device_.get_device(), 1, &frame_data_.getCurrentFrame().render_fence, true, 1000000000) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence timeout excedeed!");
#endif // !NDEBUG
	}

	//Reseteamos el fence
	if (vkResetFences(device_.get_device(), 1, &frame_data_.getCurrentFrame().render_fence) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence restart failed!");
#endif // !NDEBUG
	}

	//Solicitamos una imagen del swap chain
	//uint32_t swap_chain_image_index;
	if (vkAcquireNextImageKHR(device_.get_device(), swap_chain_.get_swap_chain(), 1000000000,
		frame_data_.getCurrentFrame().swap_chain_semaphore, nullptr, &swap_chain_image_index) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Swapchain image not retrieved!");
#endif // !NDEBUG
	}

	//Se resetea e inicia el command buffer del frame actual
	//VkCommandBuffer commandBuffer = frame_data_.getCurrentFrame().main_command_buffer;
	commandBuffer = frame_data_.getCurrentFrame().main_command_buffer;
	if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Resseting commandbuffer failed!");
#endif // !NDEBUG
	}

	frame_data_.getCurrentFrame().last_bound_mesh.reset();

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
	// Cambiamos la imagen del swap chain para poder escribir sobre ella
	TransitionImage(commandBuffer, swap_chain_.get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//draw imgui into the swapchain image
	drawImgui(commandBuffer, swap_chain_.get_swap_chain_image_views()[swap_chain_image_index]);

	//Se cambia la imagen al layout presentable
	TransitionImage(commandBuffer, swap_chain_.get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//Se finalizar el command buffer
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("End commandbuffer failed!");
#endif // !NDEBUG
	}

	VkCommandBufferSubmitInfo commandSubmitInfo = vkinit::CommandBufferSubmitInfo(commandBuffer);
	VkSemaphoreSubmitInfo waitInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame_data_.getCurrentFrame().swap_chain_semaphore);
	VkSemaphoreSubmitInfo signalInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame_data_.getCurrentFrame().render_semaphore);

	VkSubmitInfo2 submit = vkinit::SubmitInfo(&commandSubmitInfo, &signalInfo, &waitInfo);
	vkQueueSubmit2(device_.get_graphics_queue(), 1, &submit, frame_data_.getCurrentFrame().render_fence);

	//Se crea la estructura de presentacion para enviarla a la ventana de GLFW
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	VkSwapchainKHR aux_swap = swap_chain_.get_swap_chain();
	presentInfo.pSwapchains = &aux_swap;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &frame_data_.getCurrentFrame().render_semaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swap_chain_image_index;

	vkQueuePresentKHR(device_.get_present_queue(), &presentInfo);

	//increase the number of frames drawn
	frame_data_.increaseFrameNumber();

	//End Frame Callbacks(NEEDS A WRAPER)
	for (auto it = end_frame_callbacks.rbegin(); it != end_frame_callbacks.rend(); it++) {
		(*it)();
	}

	dt_ = std::chrono::duration_cast<std::chrono::microseconds>(chrono_now_ - chrono_last_update_).count() / 1000000.0f;
	chrono_last_update_ = chrono_now_;
}

void LavaEngine::mainLoop() {
	
  while (!glfwWindowShouldClose(get_window())) {

    glfwPollEvents();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		renderImgui();

		//ImGui::ShowDemoWindow();

		ImGui::Render();

		render();

		//End Frame Callbacks(NEEDS A WRAPER)
		for (auto it = end_frame_callbacks.rbegin(); it != end_frame_callbacks.rend(); it++) {
			(*it)();
		}
  }
}

void LavaEngine::clearWindow() {
	//Convertimos la imagen de dibujado a escribible
	TransitionImage(commandBuffer, swap_chain_.get_draw_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	//Limpiamos la imagen con un clear color
	VkClearColorValue clearValue;
	clearValue = { {0.0f,0.0f,0.0f,0.0f} };

	//Seleccionamos un rango de la imagen sobre la que actuar
	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//Aplicamos el clear color a una imagen 
	vkCmdClearColorImage(commandBuffer, swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

}

void LavaEngine::render() {

	TransitionImage(commandBuffer, swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	TransitionImage(commandBuffer, swap_chain_.get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	drawMeshes(commandBuffer);

	//Cambiamos tanto la imagen del swapchain como la de 
	// dibujado al mismo estado para copiar la informacion
	TransitionImage(commandBuffer, swap_chain_.get_draw_image().image ,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//Cambiamos la imagen a tipo presentable para enseñarla en la superficie
	TransitionImage(commandBuffer, swap_chain_.get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Devolvemos la imagen al swapchain
	CopyImageToImage(commandBuffer, swap_chain_.get_draw_image().image,
		swap_chain_.get_swap_chain_images()[swap_chain_image_index], swap_chain_.get_draw_extent(), window_extent_);

}

void LavaEngine::drawMeshes(VkCommandBuffer command_buffer)
{
	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(swap_chain_.get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(swap_chain_.get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	//
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(swap_chain_.get_draw_extent(), &color_attachment, &depth_attachment);
	vkCmdBeginRendering(command_buffer, &renderInfo);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)swap_chain_.get_draw_extent().width;
	viewport.height = (float)swap_chain_.get_draw_extent().height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = swap_chain_.get_draw_extent().width;
	scissor.extent.height = swap_chain_.get_draw_extent().height;

	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	//Clean Descriptor sets for current frame
	FrameData& frame_data = frame_data_.getCurrentFrame();
	frame_data.descriptor_manager.clear();

	//global_data_buffer_->updateBufferData(&global_scene_data_, sizeof(GlobalSceneData));

	GPUDrawPushConstants push_constants;
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
	model = glm::rotate(model, glm::radians(0.01f * frame_data_.frame_number_), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(0.02f * frame_data_.frame_number_), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(0.03f * frame_data_.frame_number_), glm::vec3(0.0f, 0.0f, 1.0f));

	for (auto mesh : meshes_) {

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh->get_material()->get_pipeline().get_pipeline());


		VkDescriptorSet image_set = mesh->get_material()->get_descriptor_set();
	
		//Bind both descriptor sets on the mesh
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			mesh->get_material()->get_pipeline().get_layout(),
			0, 1, &global_descriptor_set_, 0, nullptr);

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			mesh->get_material()->get_pipeline().get_layout(), 
			1, 1, &image_set, 0, nullptr);

		push_constants.world_matrix = model; // global_scene_data_.viewproj* model;
		//for (std::shared_ptr<MeshAsset> submesh : mesh->meshes_) {
		//	push_constants.vertex_buffer = submesh->meshBuffers.vertex_buffer_address;

		//	vkCmdPushConstants(command_buffer, mesh->get_material()->get_pipeline().get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		//	vkCmdBindIndexBuffer(command_buffer, submesh->meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);

		//	vkCmdDrawIndexed(command_buffer, submesh->surfaces[0].count, 1, submesh->surfaces[0].start_index, 0, 0);
		//}
	}

	vkCmdEndRendering(command_buffer);

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

	imgui_descriptor_alloc.init_pool(device_.get_device(), 1000, pool_sizes);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan(get_window(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = get_instance();
	init_info.PhysicalDevice = device_.get_physical_device();
	init_info.Device = device_.get_device();
	init_info.Queue = device_.get_graphics_queue();
	init_info.DescriptorPool = imgui_descriptor_alloc.pool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.UseDynamicRendering = true;
	init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	VkFormat swap_format = swap_chain_.get_swap_chain_image_format();
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swap_format;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();

	//main_deletion_queue_.push_function([=]() {
	//	ImGui_ImplVulkan_Shutdown();
	//	imgui_descriptor_alloc.destroy_pool(device_.get_device());
	//	});

}

void LavaEngine::immediate_submit(std::function<void(VkCommandBuffer)>&& function) {
	VkFence aux_inmediate_fence = inmediate_communication.get_inmediate_fence();
	if (vkResetFences(device_.get_device(), 1, &aux_inmediate_fence) != VK_SUCCESS) {
		exit(-1);
	}

	VkCommandBuffer aux_immediate_command_buffer = inmediate_communication.get_immediate_command_buffer();
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

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	vkQueueSubmit2(device_.get_graphics_queue(), 1, &submit, aux_inmediate_fence);

	vkWaitForFences(device_.get_device(), 1, &aux_inmediate_fence, true, 9999999999);
}

void LavaEngine::drawImgui(VkCommandBuffer command_buffer, VkImageView target_image_view) {
	VkRenderingAttachmentInfo color_attachment =
		vkinit::AttachmentInfo(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo render_info = vkinit::RenderingInfo(window_extent_, &color_attachment, nullptr);

	vkCmdBeginRendering(command_buffer, &render_info);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

	vkCmdEndRendering(command_buffer);
}

std::shared_ptr<LavaMesh> LavaEngine::addMesh(MeshProperties prop){
	std::shared_ptr<LavaMesh> mesh = std::make_shared<LavaMesh>(*this, prop);
	meshes_.push_back(mesh);
	return mesh;
}

void LavaEngine::renderImgui() {
	ImGui::ShowDemoWindow();
}
