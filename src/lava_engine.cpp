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
#include "lava_pipelines.hpp"

#define VMA_IMPLEMENTATION
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

LavaEngine::LavaEngine() : 
	surface_{ instance_.get_instance(), window_.get_window() },
	instance_{ validationLayers },
	window_{ 1280, 720, "LavaEngine" },
	device_{instance_,surface_}
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;
	is_initialized_ = false;
	frame_number_ = 0;
	stop_rendering = false;

	window_extent_ = { 1280,720 };
	swap_chain_ = VK_NULL_HANDLE;
	swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
	render_pass_ = VK_NULL_HANDLE;
}


LavaEngine::LavaEngine(unsigned int window_width, unsigned int window_height) :
	window_{window_width, window_height, "LavaEngine"},
	instance_{validationLayers},
	surface_{instance_.get_instance(), window_.get_window()},
	device_{ instance_,surface_ }
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;
	is_initialized_ = false;
	frame_number_ = 0;
	stop_rendering = false;

	window_extent_ = { window_width,window_height };
	swap_chain_ = VK_NULL_HANDLE;
	swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
	render_pass_ = VK_NULL_HANDLE;

}

LavaEngine::~LavaEngine(){
	vkDeviceWaitIdle(device_.get_device());

	main_deletion_queue_.flush();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		vkDestroyCommandPool(device_.get_device(), frames_[i].command_pool, nullptr);
		vkDestroyFence(device_.get_device(), frames_[i].render_fence, nullptr);
		vkDestroySemaphore(device_.get_device(), frames_[i].render_semaphore, nullptr);
		vkDestroySemaphore(device_.get_device(), frames_[i].swap_chain_semaphore, nullptr);
		
	}
	for (auto imageView : swap_chain_image_views_) {
		vkDestroyImageView(device_.get_device(), imageView, nullptr);
	}
	vkDestroySwapchainKHR(device_.get_device(), swap_chain_, nullptr);
	
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

		ImGui::Begin("Lava window");
		ComputeEffect& selected = backgroundEffects[currentBackgroundEffect];

		ImGui::Text("Selected effect: %s", selected.name);

		ImGui::SliderInt("Effect Index", &currentBackgroundEffect, 0, backgroundEffects.size() - 1);
		if (selected.use_push_constants) {
			ImGui::InputFloat4("data1", (float*)&selected.data.data1);
			ImGui::InputFloat4("data2", (float*)&selected.data.data2);
			ImGui::InputFloat4("data3", (float*)&selected.data.data3);
			ImGui::InputFloat4("data4", (float*)&selected.data.data4);
		}
		ImGui::End();

		//ImGui::ShowDemoWindow();

		ImGui::Render();

		draw();
  }
}

void LavaEngine::initVulkan(){

	//Despues de crear la instancia se configura el callback de las validation layers
	//Tras seleccionar el dispositivo fisico ahora toca crear el logico
	createAllocator();
	createSwapChain();
	createImageViews();
	createCommandPool();
	createSyncObjects();
	createDescriptors();
	initDefaultData();
	createPipelines();
}

void LavaEngine::createSwapChain(){
	SwapChainSupportDetails swapChainSupport =
		QuerySwapChainSupport(device_.get_physical_device(), get_surface());

	//CAREFUL 
	//En una guia recomiendan VK_FORMAT_B8G8R8A8_UNORM (https://vkguide.dev) 
	//Y en otra VK_FORMAT_B8G8R8A8_SRGV
	VkSurfaceFormatKHR surfaceFormat =
		ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		ChooseSwapPresentMode(swapChainSupport.presentModes);
	
	//VkExtent2D extent =
	//	chooseSwapExtent(swapChainSupport.capabilities, window_);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount >
		swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = get_surface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = window_extent_;
	createInfo.imageArrayLayers = 1;
	//VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	//VK_IMAGE_USAGE_TRANSFER_DST_BIT util para postprocesos
	createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(device_.get_physical_device(), get_surface());
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device_.get_device(), &createInfo,
		nullptr, &swap_chain_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!!");
	}

	vkGetSwapchainImagesKHR(device_.get_device(), swap_chain_, &imageCount, nullptr);
	swap_chain_images_.resize(imageCount);
	vkGetSwapchainImagesKHR(device_.get_device(), swap_chain_, &imageCount, swap_chain_images_.data());
	swap_chain_image_format_ = surfaceFormat.format;


	VkExtent3D draw_image_extent = {
		window_extent_.width,
		window_extent_.height,
		1
	};

	draw_image_.image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
	draw_image_.image_extent = draw_image_extent;

	draw_extent_ = window_extent_;

	VkImageUsageFlags draw_image_usages{};
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = vkinit::ImageCreateInfo(draw_image_.image_format,
		draw_image_usages, draw_image_extent);

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(allocator_, &rimg_info, &rimg_allocinfo, &draw_image_.image,
		&draw_image_.allocation, nullptr);

	VkImageViewCreateInfo rview_info = vkinit::ImageViewCreateInfo(draw_image_.image_format,
		draw_image_.image, VK_IMAGE_ASPECT_COLOR_BIT);
	if (vkCreateImageView(device_.get_device(), &rview_info, nullptr, &draw_image_.image_view) !=
		VK_SUCCESS) {
		printf("Error creating image view!\n");
	}

	main_deletion_queue_.push_function([=]() {
		vkDestroyImageView(device_.get_device(), draw_image_.image_view, nullptr);
		vmaDestroyImage(allocator_, draw_image_.image, draw_image_.allocation);
		});
}

void LavaEngine::createImageViews(){
	swap_chain_image_views_.resize(swap_chain_images_.size());
	for (size_t i = 0; i < swap_chain_images_.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swap_chain_images_[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swap_chain_image_format_;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device_.get_device(), &createInfo,
			nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}
}

void LavaEngine::createCommandPool() {
	//Primero se crean los command pool
	QueueFamilyIndices queueFamilyIndices =
		FindQueueFamilies(device_.get_physical_device(), get_surface());

	VkCommandPoolCreateInfo command_pool_info{};
	command_pool_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		if (vkCreateCommandPool(device_.get_device(), &command_pool_info, nullptr,
			&frames_[i].command_pool) != VK_SUCCESS) {
			exit(-1);
		}
		VkCommandBufferAllocateInfo command_alloc_info{};
		command_alloc_info.sType =
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_alloc_info.pNext = nullptr;
		command_alloc_info.commandPool = frames_[i].command_pool;
		command_alloc_info.commandBufferCount = 1;
		command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		//Se reserva los command buffer correspondientes
		if (vkAllocateCommandBuffers(device_.get_device(), &command_alloc_info,
			&frames_[i].main_command_buffer) != VK_SUCCESS) {
			exit(-1);
		}

	}
		
	if (vkCreateCommandPool(device_.get_device(), &command_pool_info, nullptr,
		&immediate_command_pool) != VK_SUCCESS) {
		exit(-1);
	}

		VkCommandBufferAllocateInfo immediate_command_buffer_alloc_info{};
		immediate_command_buffer_alloc_info.sType =
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		immediate_command_buffer_alloc_info.pNext = nullptr;
		immediate_command_buffer_alloc_info.commandPool = immediate_command_pool;
		immediate_command_buffer_alloc_info.commandBufferCount = 1;
		immediate_command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		//Se reserva los command buffer correspondientes
		if (vkAllocateCommandBuffers(device_.get_device(), &immediate_command_buffer_alloc_info,
			&immediate_command_buffer) != VK_SUCCESS) {
			exit(-1);
		}

		main_deletion_queue_.push_function([=]() {
			vkDestroyCommandPool(device_.get_device(), immediate_command_pool, nullptr);
			});

}

void LavaEngine::createSyncObjects() {
	//1 fence para avisar a la CPU cuando un frame ya se ha dibujado
	//1 semaforo para la comunicacion con el swapchain y otro para
	//los comandos de dibujado
	VkFenceCreateInfo fence_info{};
	fence_info.sType =
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;
	
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		if (vkCreateFence(device_.get_device(), &fence_info, nullptr, &frames_[i].render_fence) != VK_SUCCESS) {
			exit(-1);
		}
		if (vkCreateSemaphore(device_.get_device(),&semaphore_info,nullptr,&frames_[i].render_semaphore) != VK_SUCCESS) {
			exit(-1);
		}

		if (vkCreateSemaphore(device_.get_device(), &semaphore_info, nullptr, &frames_[i].swap_chain_semaphore) != VK_SUCCESS) {
			exit(-1);
		}
	}

	//Sync objects for immediate submits

	if (vkCreateFence(device_.get_device(), &fence_info, nullptr, &immediate_fence) != VK_SUCCESS) {
		exit(-1);
	}
	main_deletion_queue_.push_function([=]() {vkDestroyFence(device_.get_device(), immediate_fence, nullptr); });
}

void LavaEngine::draw() {
	//Esperamos a que el fence comunique que la grafica ya ha terminado de dibujar
	//El timeout esta en nanosegundos 10e-9
	if (vkWaitForFences(device_.get_device(), 1, &getCurrentFrame().render_fence, true, 1000000000) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence timeout excedeed!");
#endif // !NDEBUG
	}
	getCurrentFrame().deletion_queue.flush();

	//Reseteamos el fence
	if (vkResetFences(device_.get_device(), 1, &getCurrentFrame().render_fence) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence restart failed!");
#endif // !NDEBUG
	}

	//Solicitamos una imagen del swap chain
	uint32_t swap_chain_image_index;
	if (vkAcquireNextImageKHR(device_.get_device(), swap_chain_, 1000000000,
		getCurrentFrame().swap_chain_semaphore, nullptr, &swap_chain_image_index) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Swapchain image not retrieved!");
#endif // !NDEBUG
	}

	//Se resetea e inicia el command buffer del frame actual
	VkCommandBuffer commandBuffer = getCurrentFrame().main_command_buffer;
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
	TransitionImage(commandBuffer, draw_image_.image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	drawBackground(commandBuffer);

	TransitionImage(commandBuffer, draw_image_.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	DrawGeometry(commandBuffer);

	//Cambiamos tanto la imagen del swapchain como la de 
	// dibujado al mismo estado para copiar la informacion
	TransitionImage(commandBuffer,draw_image_.image ,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//Cambiamos la imagen a tipo presentable para enseñarla en la superficie
	TransitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// -> FIN PRIMER DIBUJADO
	// -> INICIO IMGUI DIBUJADO
	// Devolvemos la imagen al swapchain
	CopyImageToImage(commandBuffer, draw_image_.image, 
		swap_chain_images_[swap_chain_image_index], draw_extent_, window_extent_);

	// Cambiamos la imagen del swap chain para poder escribir sobre ella
	TransitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//draw imgui into the swapchain image
	drawImgui(commandBuffer, swap_chain_image_views_[swap_chain_image_index]);
	
	// -> FIN IMGUI DIBUJADO
	//Se cambia la imagen al layout presentable
	TransitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//Se finalizar el command buffer
if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("End commandbuffer failed!");
#endif // !NDEBUG
	}

	VkCommandBufferSubmitInfo commandSubmitInfo = vkinit::CommandBufferSubmitInfo(commandBuffer);
	VkSemaphoreSubmitInfo waitInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame().swap_chain_semaphore);
	VkSemaphoreSubmitInfo signalInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame().render_semaphore);

	VkSubmitInfo2 submit = vkinit::SubmitInfo(&commandSubmitInfo, &signalInfo, &waitInfo);
	vkQueueSubmit2(device_.get_graphics_queue(), 1, &submit, getCurrentFrame().render_fence);

	//Se crea la estructura de presentacion para enviarla a la ventana de GLFW
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swap_chain_;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &getCurrentFrame().render_semaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swap_chain_image_index;

	vkQueuePresentKHR(device_.get_present_queue(), &presentInfo);

	//increase the number of frames drawn
	frame_number_++;

}

void LavaEngine::drawBackground(VkCommandBuffer command_buffer) {

	//Se sustituye todo esto por el compute shader
//	//Limpiamos la imagen con un clear color
//	VkClearColorValue clearValue;
//	clearValue = { {1.0f,0.0f,0.0f,0.0f} };
//
//	//Seleccionamos un rango de la imagen sobre la que actuar
//	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
//
//	//Aplicamos el clear color a una imagen 
//vkCmdClearColorImage(command_buffer, draw_image_.image,
//	VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);


	//// bind the gradient drawing compute pipeline
	//vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_);
	//
	////IMPORTANT
	////Importante recordar esto cuando tengamos que hacer binding de description sets en otros shaders
	//// bind the descriptor set containing the draw image for the compute pipeline
	//vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout_, 0, 1, &draw_image_descriptor_set_, 0, nullptr);
	//
	//// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	//vkCmdDispatch(command_buffer, std::ceil(draw_image_.image_extent.width / 16.0), std::ceil(draw_image_.image_extent.height / 16.0), 1);


	ComputeEffect& effect = backgroundEffects[currentBackgroundEffect];

	// bind the background compute pipeline
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

	// bind the descriptor set containing the draw image for the compute pipeline
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, effect.layout, 0, 1, &draw_image_descriptor_set_, 0, nullptr);

	if (effect.use_push_constants) {
		vkCmdPushConstants(command_buffer, effect.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
	}
	// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	vkCmdDispatch(command_buffer, std::ceil(draw_image_.image_extent.width / 16.0), std::ceil(draw_image_.image_extent.height / 16.0), 1);

}

void LavaEngine::drawBackgroundImGui(VkCommandBuffer command_buffer) {

	//Se sustituye todo esto por el compute shader
//	//Limpiamos la imagen con un clear color
//	VkClearColorValue clearValue;
//	clearValue = { {1.0f,0.0f,0.0f,0.0f} };
//
//	//Seleccionamos un rango de la imagen sobre la que actuar
//	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
//
//	//Aplicamos el clear color a una imagen 
//vkCmdClearColorImage(command_buffer, draw_image_.image,
//	VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);


	// bind the gradient drawing compute pipeline
	//vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_);
	//
	////IMPORTANT
	////Importante recordar esto cuando tengamos que hacer binding de description sets en otros shaders
	//// bind the descriptor set containing the draw image for the compute pipeline
	//vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout_, 0, 1, &draw_image_descriptor_set_, 0, nullptr);
	//
	//ComputePushConstants pc;
	//pc.data1 = glm::vec4(1, 0, 0, 1);
	//pc.data2 = glm::vec4(0, 0, 1, 1);
	//
	//vkCmdPushConstants(command_buffer, gradient_pipeline_layout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &pc);
	//
	//// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	//vkCmdDispatch(command_buffer, std::ceil(draw_image_.image_extent.width / 16.0), std::ceil(draw_image_.image_extent.height / 16.0), 1);

}

void LavaEngine::DrawGeometry(VkCommandBuffer command_buffer)
{
	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo colorAttachment = vkinit::AttachmentInfo(draw_image_.image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(draw_extent_, &colorAttachment, nullptr);
	vkCmdBeginRendering(command_buffer, &renderInfo);
	
	//vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = draw_extent_.width;
	viewport.height = draw_extent_.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	
	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = draw_extent_.width;
	scissor.extent.height = draw_extent_.height;
	
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);

	GPUDrawPushConstants push_constants;
	push_constants.world_matrix = glm::mat4{ 1.f };
	push_constants.vertex_buffer = rectangle.vertex_buffer_address;

	vkCmdPushConstants(command_buffer, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
	vkCmdBindIndexBuffer(command_buffer, rectangle.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(command_buffer, 3, 1, 0, 0, 0);
	
	//launch a draw command to draw 3 vertices
	vkCmdDraw(command_buffer, 3, 1, 0, 0);

	vkCmdEndRendering(command_buffer);

		//launch a draw command to draw 3 vertices

}

void LavaEngine::createAllocator() {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = device_.get_physical_device();
	allocatorInfo.device = device_.get_device();
	allocatorInfo.instance = get_instance();
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &allocator_);

	main_deletion_queue_.push_function([&]() {
		vmaDestroyAllocator(allocator_);
		});
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
	img_info.imageView = draw_image_.image_view;

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

void LavaEngine::createPipelines() {
	createBackgroundPipelines();
	createBackgroundPipelinesImGui();
	createTrianglePipeline();
	createMeshPipeline();
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

void LavaEngine::createTrianglePipeline()
{
	VkShaderModule triangleFragShader;
	if (!LoadShader("../src/shaders/colored_triangle.frag.spv", device_.get_device(), &triangleFragShader)) {
		printf("Error when building the triangle fragment shader module");
	}
	else {
		printf("Triangle fragment shader succesfully loaded");
	}

	VkShaderModule triangleVertexShader;
	if (!LoadShader("../src/shaders/colored_triangle.vert.spv", device_.get_device(), &triangleVertexShader)) {
		printf("Error when building the triangle vertex shader module");
	}
	else {
		printf("Triangle vertex shader succesfully loaded");
	}

	//build the pipeline layout that controls the inputs/outputs of the shader
	//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pNext = nullptr;
	pipeline_layout_info.pSetLayouts = &draw_image_descriptor_set_layout_;
	pipeline_layout_info.setLayoutCount = 1;

	if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &_trianglePipelineLayout)) {
#ifndef NDEBUG
		printf("Compute pipeline creation failed!");
#endif // !NDEBUG
	}

	PipelineBuilder pipeline_builder;

	//use the triangle layout we created
	pipeline_builder._pipeline_layout = _trianglePipelineLayout;
	//connecting the vertex and pixel shaders to the pipeline
	pipeline_builder.SetShaders(triangleVertexShader, triangleFragShader);
	//it will draw triangles
	pipeline_builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//filled triangles
	pipeline_builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	//no backface culling
	pipeline_builder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	//no multisampling
	pipeline_builder.SetMultisamplingNone();
	//no blending
	pipeline_builder.DisableBlending();
	//no depth testing
	pipeline_builder.DisableDepthtest();

	//connect the image format we will draw into, from draw image
	pipeline_builder.SetColorAttachmentFormat(draw_image_.image_format);
	pipeline_builder.SetDepthFormat(VK_FORMAT_UNDEFINED);

	//finally build the pipeline
	_trianglePipeline = pipeline_builder.BuildPipeline(device_.get_device());

	//clean structures
	vkDestroyShaderModule(device_.get_device(), triangleFragShader, nullptr);
	vkDestroyShaderModule(device_.get_device(), triangleVertexShader, nullptr);

	main_deletion_queue_.push_function([&]() {
		vkDestroyPipelineLayout(device_.get_device(), _trianglePipelineLayout, nullptr);
		vkDestroyPipeline(device_.get_device(), _trianglePipeline, nullptr);
	});
}

void LavaEngine::createMeshPipeline()
{
	VkShaderModule triangleFragShader;
	if (!LoadShader("../src/shaders/colored_triangle.frag.spv", device_.get_device(), &triangleFragShader)) {
		printf("Error when building the triangle fragment shader module");
	}
	else {
		printf("Triangle fragment shader succesfully loaded");
	}

	VkShaderModule triangleVertexShader;
	if (!LoadShader("../src/shaders/colored_triangle_mesh.vert.spv", device_.get_device(), &triangleVertexShader)) {
		printf("Error when building the triangle vertex shader module");
	}
	else {
		printf("Triangle vertex shader succesfully loaded");
	}

	VkPushConstantRange buffer_range{};
	buffer_range.offset = 0;
	buffer_range.size = sizeof(GPUDrawPushConstants);
	buffer_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pNext = nullptr;
	pipeline_layout_info.pSetLayouts = &draw_image_descriptor_set_layout_;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pPushConstantRanges = &buffer_range;
	pipeline_layout_info.pushConstantRangeCount = 1;

	if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &_meshPipelineLayout)) {
#ifndef NDEBUG
		printf("Compute pipeline creation failed!");
#endif // !NDEBUG
	}

	PipelineBuilder pipeline_builder;

	//use the triangle layout we created
	pipeline_builder._pipeline_layout = _meshPipelineLayout;
	//connecting the vertex and pixel shaders to the pipeline
	pipeline_builder.SetShaders(triangleVertexShader, triangleFragShader);
	//it will draw triangles
	pipeline_builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//filled triangles
	pipeline_builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	//no backface culling
	pipeline_builder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	//no multisampling
	pipeline_builder.SetMultisamplingNone();
	//no blending
	pipeline_builder.DisableBlending();
	//no depth testing
	pipeline_builder.DisableDepthtest();

	//connect the image format we will draw into, from draw image
	pipeline_builder.SetColorAttachmentFormat(draw_image_.image_format);
	pipeline_builder.SetDepthFormat(VK_FORMAT_UNDEFINED);

	//finally build the pipeline
	_meshPipeline = pipeline_builder.BuildPipeline(device_.get_device());

	//clean structures
	vkDestroyShaderModule(device_.get_device(), triangleFragShader, nullptr);
	vkDestroyShaderModule(device_.get_device(), triangleVertexShader, nullptr);

	main_deletion_queue_.push_function([&]() {
		vkDestroyPipelineLayout(device_.get_device(), _meshPipelineLayout, nullptr);
		vkDestroyPipeline(device_.get_device(), _meshPipeline, nullptr);
		});
}

void LavaEngine::initDefaultData()
{
	std::array<Vertex, 3> rect_vertices;

	rect_vertices[0].position = { 0.5,0.5, 0 };
	rect_vertices[1].position = { 0.0,-0.5, 0 };
	rect_vertices[2].position = { -0.5,0.5, 0 };

	rect_vertices[0].color = { 0,0, 0,1 };
	rect_vertices[1].color = { 0.5,0.5,0.5 ,1 };
	rect_vertices[2].color = { 1,0, 0,1 };

	std::array<uint32_t, 3> rect_indices;

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rectangle = uploadMesh(rect_indices, rect_vertices);

	//delete the rectangle data on engine shutdown
	main_deletion_queue_.push_function([&]() {
		destroyBuffer(rectangle.index_buffer);
		destroyBuffer(rectangle.vertex_buffer);
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
	if (vmaCreateBuffer(allocator_, &buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation,
		&new_buffer.info)) {
#ifndef NDEBUG
		printf("Mesh Buffer creation fail!");
#endif // !NDEBUG
	}

	return new_buffer;
}

void LavaEngine::destroyBuffer(const AllocatedBuffer& buffer)
{
	vmaDestroyBuffer(allocator_, buffer.buffer, buffer.allocation);
}

GPUMeshBuffers LavaEngine::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
	const size_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
	const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers new_surface;

	//create vertex buffer
	new_surface.vertex_buffer = createBuffer(vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	//find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = new_surface.vertex_buffer.buffer };
	new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(device_.get_device(), &deviceAdressInfo);

	//create index buffer
	new_surface.index_buffer = createBuffer(index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	AllocatedBuffer staging = createBuffer(vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data = staging.allocation->GetMappedData();

	// copy vertex buffer
	memcpy(data, vertices.data(), vertex_buffer_size);
	// copy index buffer
	memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

	immediate_submit([&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertex_buffer_size;

		vkCmdCopyBuffer(cmd, staging.buffer, new_surface.vertex_buffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertex_buffer_size;
		indexCopy.size = index_buffer_size;

		vkCmdCopyBuffer(cmd, staging.buffer, new_surface.index_buffer.buffer, 1, &indexCopy);
		});

	destroyBuffer(staging);

	return new_surface;
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
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swap_chain_image_format_;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();

	main_deletion_queue_.push_function([=]() {
		ImGui_ImplVulkan_Shutdown();
		imgui_descriptor_alloc.destroy_pool(device_.get_device());
		});

}

void LavaEngine::immediate_submit(std::function<void(VkCommandBuffer)>&& function) {
	if (vkResetFences(device_.get_device(), 1, &immediate_fence) != VK_SUCCESS) {
		exit(-1);
	}

	if (vkResetCommandBuffer(immediate_command_buffer,0) != VK_SUCCESS) {
		exit(-1);
	}

	VkCommandBuffer command_buffer = immediate_command_buffer;
	VkCommandBufferBeginInfo command_buffer_begin_info = 
		vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

	function(command_buffer);

	vkEndCommandBuffer(command_buffer);

	VkCommandBufferSubmitInfo command_submit_info = vkinit::CommandBufferSubmitInfo(command_buffer);
	VkSubmitInfo2 submit = vkinit::SubmitInfo(&command_submit_info, nullptr, nullptr);

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	vkQueueSubmit2(device_.get_graphics_queue(), 1, &submit, immediate_fence);

	vkWaitForFences(device_.get_device(), 1, &immediate_fence, true, 9999999999);
}

void LavaEngine::drawImgui(VkCommandBuffer command_buffer, VkImageView target_image_view) {
	VkRenderingAttachmentInfo color_attachment =
		vkinit::AttachmentInfo(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo render_info = vkinit::RenderingInfo(window_extent_, &color_attachment, nullptr);

	vkCmdBeginRendering(command_buffer, &render_info);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

	vkCmdEndRendering(command_buffer);
}


