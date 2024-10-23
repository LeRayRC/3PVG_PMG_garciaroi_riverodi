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
#include "lava_transform.hpp"

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
	surface_{ instance_.get_instance(), window_.get_window() },
	instance_{ validationLayers },
	window_{ 1280, 720, "LavaEngine" },
	device_{ instance_,surface_ },
	window_extent_{ 1280, 720 },
	allocator_{device_, instance_},
	swap_chain_{ device_, surface_, window_extent_, allocator_.get_allocator()},
	frame_data_{device_, surface_},
	inmediate_communication{device_, surface_},
	lava_input{window_.get_window()}
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;
	is_initialized_ = false;
	stop_rendering = false;
	//render_pass_ = VK_NULL_HANDLE;
}

LavaEngine::LavaEngine(unsigned int window_width, unsigned int window_height) :
	window_{window_width, window_height, "LavaEngine"},
	instance_{validationLayers},
	surface_{instance_.get_instance(), window_.get_window()},
	device_{ instance_,surface_ },
	window_extent_{window_width, window_height},
	allocator_{ device_, instance_ },
	swap_chain_{ device_, surface_, window_extent_, allocator_.get_allocator() },
	frame_data_{ device_, surface_ },
	inmediate_communication{ device_, surface_ },
	lava_input{ window_.get_window() }
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;
	is_initialized_ = false;
	stop_rendering = false;
	//render_pass_ = VK_NULL_HANDLE;
}

LavaEngine::~LavaEngine(){
	vkDeviceWaitIdle(device_.get_device());

	//pipelines_.clear();
	main_deletion_queue_.flush();	
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

void LavaEngine::init() {
  //Solo se puede llamar una vez a la inicializacion del motor
	initVulkan();
	initImgui();
  is_initialized_ = true;
}

void LavaEngine::mainLoop() {
  while (!glfwWindowShouldClose(get_window())) {

    glfwPollEvents();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		/*ImGui::Begin("Lava window");
		ComputeEffect& selected = backgroundEffects[currentBackgroundEffect];

		ImGui::Text("Selected effect: %s", selected.name);

		ImGui::SliderInt("Effect Index", &currentBackgroundEffect, 0, backgroundEffects.size() - 1);
		if (selected.use_push_constants) {
			ImGui::InputFloat4("data1", (float*)&selected.data.data1);
			ImGui::InputFloat4("data2", (float*)&selected.data.data2);
			ImGui::InputFloat4("data3", (float*)&selected.data.data3);
			ImGui::InputFloat4("data4", (float*)&selected.data.data4);
		}
		ImGui::End();*/

		//ImGui::ShowDemoWindow();

		ImGui::Render();

		draw();

		//End Frame Callbacks(NEEDS A WRAPER)
		for (auto it = end_frame_callbacks.rbegin(); it != end_frame_callbacks.rend(); it++) {
			(*it)();
		}
  }
}

void LavaEngine::initVulkan(){

	createDescriptors();
}

void LavaEngine::draw() {
	//Esperamos a que el fence comunique que la grafica ya ha terminado de dibujar
	//El timeout esta en nanosegundos 10e-9
	if (vkWaitForFences(device_.get_device(), 1, &frame_data_.getCurrentFrame().render_fence, true, 1000000000) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence timeout excedeed!");
#endif // !NDEBUG
	}
	//getCurrentFrame().deletion_queue.flush();

	//Reseteamos el fence
	if (vkResetFences(device_.get_device(), 1, &frame_data_.getCurrentFrame().render_fence) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence restart failed!");
#endif // !NDEBUG
	}

	//Solicitamos una imagen del swap chain
	uint32_t swap_chain_image_index;
	if (vkAcquireNextImageKHR(device_.get_device(), swap_chain_.get_swap_chain(), 1000000000,
		frame_data_.getCurrentFrame().swap_chain_semaphore, nullptr, &swap_chain_image_index) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Swapchain image not retrieved!");
#endif // !NDEBUG
	}

	//Se resetea e inicia el command buffer del frame actual
	VkCommandBuffer commandBuffer = frame_data_.getCurrentFrame().main_command_buffer;
	if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Resseting commandbuffer failed!");
#endif // !NDEBUG
	}

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

	//
	//
	//A PARTIR DE AQUI SE EMPIEZA A DIBUJAR
	//
	//

	// -> INICIO PRIMER DIBUJADO
	// 
	//Convertimos la imagen de dibujado a escribible
	TransitionImage(commandBuffer, swap_chain_.get_draw_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	//Limpiamos la imagen con un clear color
	VkClearColorValue clearValue;
	clearValue = { {0.0f,0.0f,0.0f,0.0f} };

	//Seleccionamos un rango de la imagen sobre la que actuar
	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//Aplicamos el clear color a una imagen 
	vkCmdClearColorImage(commandBuffer, swap_chain_.get_draw_image().image,VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

	//drawBackground(commandBuffer);

	TransitionImage(commandBuffer, swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	TransitionImage(commandBuffer, swap_chain_.get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	//DrawGeometry(commandBuffer);
	/*if (!pipelines_.empty()) {
		DrawMesh(commandBuffer);
	}*/
	drawMeshes(commandBuffer);

	//DrawGeometryWithProperties(commandBuffer);

	//Cambiamos tanto la imagen del swapchain como la de 
	// dibujado al mismo estado para copiar la informacion
	TransitionImage(commandBuffer, swap_chain_.get_draw_image().image ,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//Cambiamos la imagen a tipo presentable para enseñarla en la superficie
	TransitionImage(commandBuffer, swap_chain_.get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// -> FIN PRIMER DIBUJADO
	// -> INICIO IMGUI DIBUJADO
	// Devolvemos la imagen al swapchain
	CopyImageToImage(commandBuffer, swap_chain_.get_draw_image().image,
		swap_chain_.get_swap_chain_images()[swap_chain_image_index], swap_chain_.get_draw_extent(), window_extent_);

	// Cambiamos la imagen del swap chain para poder escribir sobre ella
	TransitionImage(commandBuffer, swap_chain_.get_swap_chain_images()[swap_chain_image_index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//draw imgui into the swapchain image
	drawImgui(commandBuffer, swap_chain_.get_swap_chain_image_views()[swap_chain_image_index]);
	
	// -> FIN IMGUI DIBUJADO
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

}

void LavaEngine::drawMeshes(VkCommandBuffer command_buffer)
{
	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(swap_chain_.get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(swap_chain_.get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	//
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(swap_chain_.get_draw_extent(), &color_attachment, &depth_attachment);
	vkCmdBeginRendering(command_buffer, &renderInfo);

	//vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = swap_chain_.get_draw_extent().width;
	viewport.height = swap_chain_.get_draw_extent().height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = swap_chain_.get_draw_extent().width;
	scissor.extent.height = swap_chain_.get_draw_extent().height;

	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	for (auto mesh : meshes_) {

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh->get_material()->get_pipeline().get_pipeline());

		GPUDrawPushConstants push_constants;


		glm::mat4 model = glm::mat4(1.0f);
		LavaTransform& transform = mesh->get_transform();

		model = LavaTransform::TranslationMatrix(model, transform);
		model = LavaTransform::RotateMatrix(model, transform);
		model = LavaTransform::ScaleMatrix(model, transform);

		glm::mat4 projection = glm::perspective(glm::radians(70.0f),
			(float)swap_chain_.get_draw_extent().width/ (float)swap_chain_.get_draw_extent().height,10000.f,0.1f);

		projection[1][1] *= -1;


		push_constants.world_matrix = projection * model;
		for (std::shared_ptr<MeshAsset> submesh : mesh->meshes_) {
			push_constants.vertex_buffer = submesh->meshBuffers.vertex_buffer_address;

			vkCmdPushConstants(command_buffer, mesh->get_material()->get_pipeline().get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
			vkCmdBindIndexBuffer(command_buffer, submesh->meshBuffers.index_buffer.buffer , 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(command_buffer, submesh->surfaces[0].count, 1, submesh->surfaces[0].start_index, 0, 0);
		}
	}

	vkCmdEndRendering(command_buffer);

}

void LavaEngine::createDescriptors() {
	//Se crean 10 descriptor sets, cada uno con una imagen
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
	};

	global_descriptor_allocator_.init_pool(device_.get_device(), 10, sizes);

	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		draw_image_descriptor_set_layout_ = builder.build(device_.get_device(), VK_SHADER_STAGE_COMPUTE_BIT);
	}

	//Ahora se reserva el description set
	draw_image_descriptor_set_ =
		global_descriptor_allocator_.allocate(device_.get_device(), draw_image_descriptor_set_layout_);

	VkDescriptorImageInfo img_info{};
	img_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	//La imagen que se creo con anterioridad 
	// fuera del swap chain para poder dibujar sobre ella
	img_info.imageView = swap_chain_.get_draw_image().image_view;

	VkWriteDescriptorSet draw_image_write{};
	draw_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	draw_image_write.pNext = nullptr;

	draw_image_write.dstBinding = 0;
	draw_image_write.dstSet = draw_image_descriptor_set_;
	draw_image_write.descriptorCount = 1;
	draw_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	draw_image_write.pImageInfo = &img_info;

	vkUpdateDescriptorSets(device_.get_device(), 1, &draw_image_write, 0, nullptr);
	
	//make sure both the descriptor allocator and the new layout get cleaned up properly
	main_deletion_queue_.push_function([&]() {
		global_descriptor_allocator_.destroy_pool(device_.get_device());
		vkDestroyDescriptorSetLayout(device_.get_device(), draw_image_descriptor_set_layout_, nullptr);
		});
}

void LavaEngine::createBackgroundPipelines() {
	VkPipelineLayoutCreateInfo compute_layout{};
	compute_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	compute_layout.pNext = nullptr;
	compute_layout.pSetLayouts = &draw_image_descriptor_set_layout_;
	compute_layout.setLayoutCount = 1;
	ComputeEffect gradient;
	if (vkCreatePipelineLayout(device_.get_device(), &compute_layout, nullptr, &gradient.layout) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Pipeline layout creation failed!");
#endif // !NDEBUG
	}

	//layout code
	VkShaderModule compute_draw_shader;
	if (!LoadShader("../src/shaders/gradient.comp.spv", device_.get_device(), &compute_draw_shader))
	{
		printf("Error when building the compute shader \n");
	}

	VkPipelineShaderStageCreateInfo stage_info{};
	stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_info.pNext = nullptr;
	stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage_info.module = compute_draw_shader;
	stage_info.pName = "main";

	VkComputePipelineCreateInfo compute_pipeline_create_info{};
	compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compute_pipeline_create_info.pNext = nullptr;
	compute_pipeline_create_info.layout = gradient.layout;
	compute_pipeline_create_info.stage = stage_info;

	
	//gradient.layout = gradient_pipeline_layout_;
	gradient.name = "basic gradient";
	gradient.data = {};
	gradient.use_push_constants = false;

	if(vkCreateComputePipelines(device_.get_device(), VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &gradient.pipeline) != VK_SUCCESS){
#ifndef NDEBUG
		printf("Compute pipeline creation failed!");
#endif // !NDEBUG
	}

	backgroundEffects.push_back(gradient);

	vkDestroyShaderModule(device_.get_device(), compute_draw_shader, nullptr);
	main_deletion_queue_.push_function([&]() {
		//vkDestroyPipelineLayout(device_, gradient_pipeline_layout_, nullptr);
		vkDestroyPipelineLayout(device_.get_device(), backgroundEffects[0].layout, nullptr);
		vkDestroyPipeline(device_.get_device(), backgroundEffects[0].pipeline, nullptr);
		});
}

void LavaEngine::createBackgroundPipelinesImGui()
{
	VkPipelineLayoutCreateInfo compute_layout{};
	compute_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	compute_layout.pNext = nullptr;
	compute_layout.pSetLayouts = &draw_image_descriptor_set_layout_;
	compute_layout.setLayoutCount = 1;

	VkPushConstantRange push_constant{};
	push_constant.offset = 0;
	push_constant.size = sizeof(ComputePushConstants);
	push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	compute_layout.pPushConstantRanges = &push_constant;
	compute_layout.pushConstantRangeCount = 1;
	ComputeEffect gradient_imgui;
	if (vkCreatePipelineLayout(device_.get_device(), &compute_layout, nullptr, &gradient_imgui.layout) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Pipeline layout creation failed!");
#endif // !NDEBUG
	}

	//layout code
	VkShaderModule compute_draw_shader;
	if (!LoadShader("../src/shaders/gradient_imgui.comp.spv", device_.get_device(), &compute_draw_shader))
	{
		printf("Error when building the compute shader \n");
	}

	VkPipelineShaderStageCreateInfo stage_info{};
	stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_info.pNext = nullptr;
	stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage_info.module = compute_draw_shader;
	stage_info.pName = "main";

	VkComputePipelineCreateInfo compute_pipeline_create_info{};
	compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compute_pipeline_create_info.pNext = nullptr;
	compute_pipeline_create_info.layout = gradient_imgui.layout;
	compute_pipeline_create_info.stage = stage_info;

	
	//gradient_imgui.layout = gradient_imgui_pipeline_layout_;
	gradient_imgui.name = "ImGui gradient";
	gradient_imgui.data = {};
	gradient_imgui.use_push_constants = true;

	//default colors
	gradient_imgui.data.data1 = glm::vec4(1, 0, 0, 1);
	gradient_imgui.data.data2 = glm::vec4(0, 0, 1, 1);

	if (vkCreateComputePipelines(device_.get_device(), VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &gradient_imgui.pipeline) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Compute pipeline creation failed!");
#endif // !NDEBUG
	}

	backgroundEffects.push_back(gradient_imgui);

	vkDestroyShaderModule(device_.get_device(), compute_draw_shader, nullptr);
	main_deletion_queue_.push_function([&]() {
		//vkDestroyPipelineLayout(device_, gradient_imgui_pipeline_layout_, nullptr);
		vkDestroyPipelineLayout(device_.get_device(), backgroundEffects[1].layout, nullptr);
		vkDestroyPipeline(device_.get_device(), backgroundEffects[1].pipeline, nullptr);
		});
}

AllocatedBuffer LavaEngine::createBuffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
{
	// allocate buffer
	VkBufferCreateInfo buffer_info = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.pNext = nullptr;
	buffer_info.size = alloc_size;

	buffer_info.usage = usage;

	VmaAllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = memory_usage;
	vmaalloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	AllocatedBuffer new_buffer;

	// allocate the buffer
	if (vmaCreateBuffer(allocator_.get_allocator(), &buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation,
		&new_buffer.info)) {
#ifndef NDEBUG
		printf("Mesh Buffer creation fail!");
#endif // !NDEBUG
	}

	return new_buffer;
}

void LavaEngine::destroyBuffer(const AllocatedBuffer& buffer)
{
	vmaDestroyBuffer(allocator_.get_allocator(), buffer.buffer, buffer.allocation);
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

	main_deletion_queue_.push_function([=]() {
		ImGui_ImplVulkan_Shutdown();
		imgui_descriptor_alloc.destroy_pool(device_.get_device());
		});

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
