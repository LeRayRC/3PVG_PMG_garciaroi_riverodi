#include "lava/ecs/lava_deferred_render_system.hpp"

#include "lava/engine/lava_engine.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_allocator.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/common/lava_shapes.hpp"

LavaDeferredRenderSystem::LavaDeferredRenderSystem(LavaEngine& engine) :
	engine_{ engine },
	pipeline_geometry_pass_{ std::make_unique<LavaPipeline>(PipelineConfig(
							PIPELINE_TYPE_PBR,
							"../src/shaders/deferred/geometry_pass.vert.spv",
							"../src/shaders/deferred/geometry_pass.frag.spv",
							engine_.device_.get(),
							engine_.swap_chain_.get(),
							engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
							PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO,
							nullptr,gbuffer_count)
							) },
	pipeline_light_pass_{ std::make_unique<LavaPipeline>(PipelineConfig(
							PIPELINE_TYPE_PBR,
							"../src/shaders/deferred/light_pass.vert.spv",
							"../src/shaders/deferred/light_pass.frag.spv",
							engine_.device_.get(),
							engine_.swap_chain_.get(),
							engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
							PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO)) },
	light_pass_material{ engine_, MaterialPBRProperties()}
{

	{

		VkExtent3D draw_image_extent = {
		4096,
		4096,
		1
		};

		VmaAllocationCreateInfo rimg_allocinfo = {};
		rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		for (int i = 0; i < 3; i++) {
			//Create shadow map on the swapchain
			shadowmap_image_[i].image_format = VK_FORMAT_D32_SFLOAT;
			shadowmap_image_[i].image_extent = draw_image_extent;
			VkImageUsageFlags shadowmap_image_usages{};
			shadowmap_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			shadowmap_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

			int layers = 1;
			if (i == 0) layers = 3;
			else if (i == 1)layers = 6;

			VkImageCreateInfo shadowmap_img_info = vkinit::ImageCreateInfo(shadowmap_image_[i].image_format,
				shadowmap_image_usages, draw_image_extent, layers);

			//allocate and create the image
			vmaCreateImage(engine_.allocator_->get_allocator(), &shadowmap_img_info, &rimg_allocinfo, &shadowmap_image_[i].image, &shadowmap_image_[i].allocation, nullptr);

			//build a image-view for the draw image to use for rendering
			VkImageViewCreateInfo shadowmap_view_info = vkinit::ImageViewCreateInfo(shadowmap_image_[i].image_format, shadowmap_image_[i].image, VK_IMAGE_ASPECT_DEPTH_BIT, layers);

			if (vkCreateImageView(engine.device_->get_device(), &shadowmap_view_info, nullptr, &shadowmap_image_[i].image_view) !=
				VK_SUCCESS) {
				printf("Error creating shadowmap image view!\n");
			}

			VkSamplerCreateInfo sampler_info{};
			sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			sampler_info.unnormalizedCoordinates = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE; // Sin PCF
			sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_info.mipLodBias = 0.0f;
			sampler_info.minLod = 0.0f;
			sampler_info.maxLod = 0.0f; // Sin mipmaps


			vkCreateSampler(engine.device_->get_device(), &sampler_info, nullptr, &shadowmap_sampler_[i]);

			
		}

	}


	VkImageUsageFlags draw_image_usages{};
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkExtent3D gbuffer_extent = VkExtent3D(engine.window_extent_.width, engine.window_extent_.height, 1);

	for (int i = 0; i < gbuffer_count; i++) {
		
		//VK_FORMAT_R32G32B32A32_SFLOAT
		gbuffers_[i] = std::make_shared<LavaImage>(&engine_,
			gbuffer_extent,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			draw_image_usages);

	}

	//Create Quad
	light_pass_quad_ = CreateQuad(engine_, &light_pass_material);
	light_pass_material.UpdateBaseColorImage(gbuffers_[1]);
	light_pass_material.UpdateNormalImage(gbuffers_[2]);
}


void LavaDeferredRenderSystem::render(
	std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector
) {

	VkCommandBuffer cmd = engine_.commandBuffer;
	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();


	pipelineBarrierForRenderPassStart(cmd);
	AdvancedTransitionImage(engine_.commandBuffer, 
		engine_.swap_chain_->get_depth_image().image, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);
	//TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	std::vector<VkRenderingAttachmentInfo> color_attachments;
	for (auto& gbuffer_image : gbuffers_) {
		VkRenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		attachmentInfo.imageView = gbuffer_image->get_allocated_image().image_view;
		attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentInfo.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		color_attachments.push_back(attachmentInfo);
	}



	{

		VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_->get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
		//VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(shadowmap_image_[0].image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);

		VkRenderingInfo renderInfo = vkinit::RenderingInfo(
			engine_.swap_chain_->get_draw_extent(), 
			color_attachments.data(), 
			static_cast<uint32_t>(color_attachments.size()), 
			&depth_attachment);


		vkCmdBeginRendering(cmd, &renderInfo);

		engine_.setDynamicViewportAndScissor(engine_.swap_chain_->get_draw_extent());
	}

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_geometry_pass_->get_pipeline());
	//Bind both descriptor sets on the mesh
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_geometry_pass_->get_layout(),
		0, 1, &engine_.global_descriptor_set_, 0, nullptr);


	
	//Draw everycomponent
	auto transform_it = transform_vector.begin();
	auto render_it = render_vector.begin();
	auto transform_end = transform_vector.end();
	auto render_end = render_vector.end();
	for (; transform_it != transform_end || render_it != render_end; transform_it++, render_it++) {
		if (!transform_it->has_value()) continue;
		if (!render_it->has_value()) continue;

		if (render_it->value().active_ != RenderType_UNLIT) continue;

		//Clean Descriptor sets for current frame
		frame_data.descriptor_manager.clear();

		std::shared_ptr<LavaMesh> lava_mesh = render_it->value().mesh_;
		std::shared_ptr<MeshAsset> mesh = lava_mesh->mesh_;

		VkDescriptorSet pbr_descriptor_set = lava_mesh->get_material()->get_descriptor_set();
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_geometry_pass_->get_layout(),
			1, 1, &pbr_descriptor_set, 0, nullptr);

		GPUDrawPushConstants push_constants;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, transform_it->value().pos_);
		model = glm::rotate(model, glm::radians(transform_it->value().rot_.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(transform_it->value().rot_.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(transform_it->value().rot_.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, transform_it->value().scale_);

		// Vincular los Vertex y Index Buffers
		GPUMeshBuffers& meshBuffers = mesh->meshBuffers;
		VkDeviceSize offsets[] = { 0 };
		if (frame_data.last_bound_mesh != lava_mesh) {
			vkCmdBindIndexBuffer(cmd, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		}

		push_constants.world_matrix = model;
		push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

		vkCmdPushConstants(cmd, pipeline_geometry_pass_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		vkCmdDrawIndexed(cmd, mesh->index_count, 1, 0, 0, 0);

		if (frame_data.last_bound_mesh != lava_mesh) {
			frame_data.last_bound_mesh = lava_mesh;
		}
	}

	vkCmdEndRendering(cmd);


	pipelineBarrierForRenderPassEnd(cmd);
	AdvancedTransitionImage(cmd, engine_.swap_chain_->get_draw_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//Light Pass 
	{

		VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_->get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
		VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_->get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		//VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(shadowmap_image_[0].image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);

		VkRenderingInfo renderInfo = vkinit::RenderingInfo(
			engine_.swap_chain_->get_draw_extent(),
			&color_attachment,
			1,
			&depth_attachment);

		vkCmdBeginRendering(cmd, &renderInfo);

		engine_.setDynamicViewportAndScissor(engine_.swap_chain_->get_draw_extent());
	}

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_light_pass_->get_pipeline());
	//Bind both descriptor sets on the mesh
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_light_pass_->get_layout(),
		0, 1, &engine_.global_descriptor_set_, 0, nullptr);

	VkDescriptorSet quad_descriptor_set = light_pass_quad_->get_material()->get_descriptor_set();
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_light_pass_->get_layout(),
		1, 1, &quad_descriptor_set, 0, nullptr);


	GPUDrawPushConstants push_constants;
	glm::mat4 model = glm::mat4(1.0f);

	// Vincular los Vertex y Index Buffers
	GPUMeshBuffers& meshBuffers = light_pass_quad_->mesh_->meshBuffers;
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindIndexBuffer(cmd, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
	

	push_constants.world_matrix = model;
	push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

	vkCmdPushConstants(cmd, pipeline_geometry_pass_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
	vkCmdDrawIndexed(cmd, light_pass_quad_->mesh_->index_count, 1, 0, 0, 0);

	vkCmdEndRendering(cmd);



	AdvancedTransitionImage(cmd, engine_.swap_chain_->get_draw_image().image,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	AdvancedTransitionImage(cmd, engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


	CopyImageToImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image,
		engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_->get_draw_extent(), engine_.window_extent_);

}

LavaDeferredRenderSystem::~LavaDeferredRenderSystem()
{
	for (int i = 0; i < 3; i++) {
		vkDestroySampler(engine_.device_->get_device(), shadowmap_sampler_[i], nullptr);
		vkDestroyImageView(engine_.device_->get_device(), shadowmap_image_[i].image_view, nullptr);
		vmaDestroyImage(engine_.allocator_->get_allocator(), shadowmap_image_[i].image, shadowmap_image_[i].allocation);
	}

}


void LavaDeferredRenderSystem::setupGBufferBarriers(VkCommandBuffer cmd, VkImageLayout newLayout) {
	VkImageLayout oldLayout = (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ?
		VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkImageMemoryBarrier2> barriers;
	barriers.reserve(gbuffer_count);

	for (int i = 0; i < gbuffer_count; i++) {
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = gbuffers_[i]->get_allocated_image().image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barrier.srcAccessMask = 0;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		}
		else {
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
		}

		barriers.push_back(barrier);
	}

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
	dependencyInfo.pImageMemoryBarriers = barriers.data();

	vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

void LavaDeferredRenderSystem::setupShadowMapBarriers(VkCommandBuffer cmd, VkImageLayout newLayout) {
	std::vector<VkImageMemoryBarrier2> barriers;
	barriers.reserve(3);

	for (int i = 0; i < 3; i++) {
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}

		barrier.newLayout = newLayout;
		barrier.image = shadowmap_image_[i].image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Solo depth, sin stencil
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = (i == 0) ? 3 : (i == 1) ? 6 : 1;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barrier.srcAccessMask = 0;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else {
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
		}

		barriers.push_back(barrier);
	}

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
	dependencyInfo.pImageMemoryBarriers = barriers.data();

	vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

void LavaDeferredRenderSystem::pipelineBarrierForRenderPassStart(VkCommandBuffer cmd) {
	// Primero las shadow maps para que estén listas cuando las necesitemos
	setupShadowMapBarriers(cmd, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	// Luego los G-Buffers
	setupGBufferBarriers(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void LavaDeferredRenderSystem::pipelineBarrierForRenderPassEnd(VkCommandBuffer cmd) {
	// Primero los G-Buffers para que estén listos para el light pass
	setupGBufferBarriers(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Luego las shadow maps
	setupShadowMapBarriers(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}