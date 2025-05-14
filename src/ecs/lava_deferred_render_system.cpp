#include "lava/ecs/lava_deferred_render_system.hpp"

#include "lava/engine/lava_engine.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_allocator.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/common/lava_shapes.hpp"
#include "lava/common/lava_global_helpers.hpp"

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
	pipeline_light_pass_first_{ std::make_unique<LavaPipeline>(PipelineConfig(
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
							PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO,
							nullptr,1))
							},
	pipeline_light_pass_additive_{ std::make_unique<LavaPipeline>(PipelineConfig(
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
							PipelineBlendMode::PIPELINE_BLEND_ONE_ONE,
							nullptr,1))
							},
	pipeline_light_pass_ambient_{ std::make_unique<LavaPipeline>(PipelineConfig(
							PIPELINE_TYPE_PBR,
							"../src/shaders/deferred/light_pass.vert.spv",
							"../src/shaders/deferred/light_pass_ambient.frag.spv",
							engine_.device_.get(),
							engine_.swap_chain_.get(),
							engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
							PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ONE,
							nullptr,1))
							},
	pipeline_shadows_{ std::make_unique<LavaPipeline>(PipelineConfig( //TO DO: DIRECTIONAL LIGHT
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/point_shadow_mapping.vert.spv", 
													"../src/shaders/shadow_mapping.frag.spv",
													engine_.device_.get(),
													engine_.swap_chain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO,
													"../src/shaders/directional_shadows.geom.spv")),

						 std::make_unique<LavaPipeline>(PipelineConfig(
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/point_shadow_mapping.vert.spv",
													"../src/shaders/point_shadow_mapping.frag.spv",
													engine_.device_.get(),
													engine_.swap_chain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO,
													"../src/shaders/point_shadows.geom.spv")),

						 std::make_unique<LavaPipeline>(PipelineConfig(
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/shadow_mapping.vert.spv",
													"../src/shaders/shadow_mapping.frag.spv",
													engine_.device_.get(),
													engine_.swap_chain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO)), },
	light_pass_material{std::make_shared<LavaPBRMaterial>(engine_, MaterialPBRProperties())}
{

	VkExtent3D shadowmap_image_extent = {4096,4096,1};

	VkImageUsageFlags shadowmap_image_usages{};
	shadowmap_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	shadowmap_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkSamplerCreateInfo shadowmap_sampler_info{};
	shadowmap_sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	shadowmap_sampler_info.magFilter = VK_FILTER_LINEAR;
	shadowmap_sampler_info.minFilter = VK_FILTER_LINEAR;
	shadowmap_sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	shadowmap_sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	shadowmap_sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	shadowmap_sampler_info.anisotropyEnable = VK_FALSE;
	shadowmap_sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	shadowmap_sampler_info.unnormalizedCoordinates = VK_FALSE;
	shadowmap_sampler_info.compareEnable = VK_FALSE;
	shadowmap_sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	shadowmap_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	shadowmap_sampler_info.mipLodBias = 0.0f;
	shadowmap_sampler_info.minLod = 0.0f;
	shadowmap_sampler_info.maxLod = 0.0f;

	for (int i = 0; i < 3; i++) {

		int layers = 1;
		if (i == 0) layers = 3;
		else if (i == 1)layers = 6;

		shadowmaps_[i] = std::make_shared<LavaImage>(&engine_,
			shadowmap_image_extent,
			VK_FORMAT_D32_SFLOAT,
			shadowmap_image_usages,
			false,
			layers,
			&shadowmap_sampler_info
			);
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
	light_pass_quad_ = CreateQuad(light_pass_material);
	light_pass_material->UpdatePositionImage(gbuffers_[0]);
	light_pass_material->UpdateBaseColorImage(gbuffers_[1]);
	light_pass_material->UpdateNormalImage(gbuffers_[2]);

}


void LavaDeferredRenderSystem::render(
	std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector
) {

	VkCommandBuffer cmd = engine_.commandBuffer;
	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();


	allocateLights(light_component_vector);
	updateLights(light_component_vector, transform_vector);

	renderGeometryPass(transform_vector, render_vector, light_component_vector);
	renderShadowMaps(transform_vector, render_vector, light_component_vector);
	renderLightPass(transform_vector, render_vector, light_component_vector);
	renderAmbient();

	//Return image to swapchain
	AdvancedTransitionImage(cmd, engine_.swap_chain_->get_draw_image().image,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	AdvancedTransitionImage(cmd, engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	CopyImageToImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image,
		engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_->get_draw_extent(), engine_.window_extent_);

}

void LavaDeferredRenderSystem::renderAmbient() {
	VkCommandBuffer cmd = engine_.commandBuffer;
	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();

	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_->get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_->get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = vkinit::RenderingInfo(
		engine_.swap_chain_->get_draw_extent(),
		&color_attachment,
		&depth_attachment);

	vkCmdBeginRendering(cmd, &renderInfo);

	engine_.setDynamicViewportAndScissor(engine_.swap_chain_->get_draw_extent());

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_light_pass_first_->get_layout(),
		0, 1, &engine_.global_descriptor_set_, 0, nullptr);

	VkDescriptorSet quad_descriptor_set = light_pass_quad_->get_material()->get_descriptor_set();
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_light_pass_first_->get_layout(),
		1, 1, &quad_descriptor_set, 0, nullptr);

	vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_light_pass_ambient_->get_pipeline());

	GPUDrawPushConstants push_constants;
	glm::mat4 model = glm::mat4(1.0f);

	// Vincular los Vertex y Index Buffers
	GPUMeshBuffers& meshBuffers = light_pass_quad_->mesh_->meshBuffers;
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindIndexBuffer(cmd, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);


	push_constants.world_matrix = model;
	push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

	vkCmdPushConstants(cmd, pipeline_light_pass_ambient_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
	vkCmdDrawIndexed(cmd, light_pass_quad_->mesh_->index_count, 1, 0, 0, 0);

	vkCmdEndRendering(cmd);
}

void LavaDeferredRenderSystem::renderShadowMaps(std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector) {

	VkCommandBuffer cmd = engine_.commandBuffer;
	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();



	setupShadowMapBarriers(cmd, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	/*for (int i = 0; i < 3; i++) {
		TransitionImage(engine_.commandBuffer,
			shadowmaps_[i]->get_allocated_image().image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, true);
	}*/

	auto light_transform_it = transform_vector.begin();
	auto light_transform_end = transform_vector.end();
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	for (; light_transform_it != light_transform_end || light_it != light_end; light_transform_it++, light_it++)
	{
		if (!light_transform_it->has_value()) continue;
		if (!light_it->has_value()) continue;

		if (!light_it->value().enabled_) continue;

		int light_index = (int)light_it->value().type_;
		VkImage current_shadowmap = shadowmaps_[light_index]->get_allocated_image().image;
		VkImageView current_shadowmap_image_view = shadowmaps_[light_index]->get_allocated_image().image_view;
		VkExtent2D current_extent;
		current_extent.width = shadowmaps_[light_index]->get_allocated_image().image_extent.width;
		current_extent.height = shadowmaps_[light_index]->get_allocated_image().image_extent.height;

		VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(current_shadowmap_image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
		int layers = 1;
		if (light_index == 0) layers = 3;
		else if (light_index == 1)layers = 6;
		VkRenderingInfo renderInfo = vkinit::RenderingInfo(current_extent, nullptr, &depth_attachment, layers);

		vkCmdBeginRendering(engine_.commandBuffer, &renderInfo);
		engine_.setDynamicViewportAndScissor(current_extent);
		

		//First Draw Create Shadow Map
		LavaPipeline* active_pipeline = pipeline_shadows_[light_index].get();
		vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());

		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			0, 1, &engine_.global_descriptor_set_, 0, nullptr);

		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			2, 1, &light_it->value().descriptor_set_, 0, nullptr);

		auto transform_it = transform_vector.begin();
		auto render_it = render_vector.begin();
		auto transform_end = transform_vector.end();
		auto render_end = render_vector.end();
		for (; transform_it != transform_end || render_it != render_end; transform_it++, render_it++) {
			if (!transform_it->has_value()) continue;
			if (!render_it->has_value()) continue;

			if (render_it->value().render_type_ != RenderType_LIT) continue;

			std::shared_ptr<LavaMesh> lava_mesh = render_it->value().mesh_;
			std::shared_ptr<MeshAsset> mesh = lava_mesh->mesh_;

			VkDescriptorSet pbr_descriptor_set = lava_mesh->get_material()->get_descriptor_set();
			vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				active_pipeline->get_layout(),
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
				vkCmdBindIndexBuffer(engine_.commandBuffer, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
			}

			push_constants.world_matrix = model;
			push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

			vkCmdPushConstants(engine_.commandBuffer, active_pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
			vkCmdDrawIndexed(engine_.commandBuffer, mesh->index_count, 1, 0, 0, 0);

			if (frame_data.last_bound_mesh != lava_mesh) {
				frame_data.last_bound_mesh = lava_mesh;
			}
		}

		vkCmdEndRendering(engine_.commandBuffer);


	}


	/*for (int i = 0; i < 3; i++) {
		TransitionImage(engine_.commandBuffer,
			shadowmaps_[i]->get_allocated_image().image,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, true);
	}*/
	setupShadowMapBarriers(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void LavaDeferredRenderSystem::renderLightPass(std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector) {

	VkCommandBuffer cmd = engine_.commandBuffer;
	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();
	
	LavaPipeline* binded_pipeline = nullptr;
	LavaPipeline* active_pipeline = pipeline_light_pass_first_.get();


	AdvancedTransitionImage(cmd, engine_.swap_chain_->get_draw_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_->get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_->get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = vkinit::RenderingInfo(
		engine_.swap_chain_->get_draw_extent(),
		&color_attachment,
		&depth_attachment);

	vkCmdBeginRendering(cmd, &renderInfo);

	engine_.setDynamicViewportAndScissor(engine_.swap_chain_->get_draw_extent());

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_light_pass_first_->get_layout(),
		0, 1, &engine_.global_descriptor_set_, 0, nullptr);

	VkDescriptorSet quad_descriptor_set = light_pass_quad_->get_material()->get_descriptor_set();
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_light_pass_first_->get_layout(),
		1, 1, &quad_descriptor_set, 0, nullptr);

	auto light_transform_it = transform_vector.begin();
	auto light_transform_end = transform_vector.end();
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	//Render each light, first one is one_zero and next one are additive
	for (; light_transform_it != light_transform_end || light_it != light_end; light_transform_it++, light_it++)
	{
		if (!light_transform_it->has_value()) continue;
		if (!light_it->has_value()) continue;

		if (!light_it->value().enabled_) continue;
	
		if (active_pipeline != binded_pipeline) {
			vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());
			binded_pipeline = active_pipeline;
		}

		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			2, 1, &light_it->value().descriptor_set_, 0, nullptr);


		GPUDrawPushConstants push_constants;
		glm::mat4 model = glm::mat4(1.0f);

		// Vincular los Vertex y Index Buffers
		GPUMeshBuffers& meshBuffers = light_pass_quad_->mesh_->meshBuffers;
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindIndexBuffer(cmd, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);


		push_constants.world_matrix = model;
		push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

		vkCmdPushConstants(cmd, active_pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		vkCmdDrawIndexed(cmd, light_pass_quad_->mesh_->index_count, 1, 0, 0, 0);


		active_pipeline = pipeline_light_pass_additive_.get();
	}

	vkCmdEndRendering(cmd);
}


void LavaDeferredRenderSystem::renderGeometryPass(
	std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector)
{

	VkCommandBuffer cmd = engine_.commandBuffer;
	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();

	setupGBufferBarriers(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	AdvancedTransitionImage(engine_.commandBuffer,
		engine_.swap_chain_->get_depth_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);

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

	

	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(
		engine_.swap_chain_->get_depth_image().image_view, 
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, 
		VK_ATTACHMENT_LOAD_OP_CLEAR);
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(
		engine_.swap_chain_->get_draw_extent(),
		color_attachments.data(),
		static_cast<uint32_t>(color_attachments.size()),
		&depth_attachment);

	vkCmdBeginRendering(cmd, &renderInfo);
	engine_.setDynamicViewportAndScissor(engine_.swap_chain_->get_draw_extent());
	

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
		//VkDeviceSize offsets[] = { 0 };
		//if (frame_data.last_bound_mesh != lava_mesh) {
		//	vkCmdBindIndexBuffer(cmd, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		//}

		//push_constants.world_matrix = model;
		//push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

		//vkCmdPushConstants(cmd, pipeline_geometry_pass_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		//vkCmdDrawIndexed(cmd, mesh->index_count, 1, 0, 0, 0);

		if (frame_data.last_bound_mesh != lava_mesh) {
			vkCmdBindIndexBuffer(engine_.commandBuffer, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
			frame_data.last_bound_mesh = lava_mesh;
		}

		// Dibujar cada superficie con su material correspondiente
		for (const GeoSurface& surface : mesh->surfaces) {
			// Obtener el material para esta superficie
			std::shared_ptr<LavaPBRMaterial> material;

			if (surface.material_index < lava_mesh->materials_.size()) {
				material = lava_mesh->materials_[surface.material_index];
			}
			else {
				//material = lava_mesh->get_material(); // Material por defecto
			}

			// Vincular descriptor set del material
			VkDescriptorSet pbr_descriptor_set = material->get_descriptor_set();
			vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeline_geometry_pass_->get_layout(),
				1, 1, &pbr_descriptor_set, 0, nullptr);

			// Configurar push constants
			GPUDrawPushConstants push_constants;
			push_constants.world_matrix = model;
			push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

			vkCmdPushConstants(engine_.commandBuffer, pipeline_geometry_pass_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);

			// Dibujar la superficie
			vkCmdDrawIndexed(engine_.commandBuffer, surface.count, 1, surface.start_index, 0, 0);
		}

		if (frame_data.last_bound_mesh != lava_mesh) {
			frame_data.last_bound_mesh = lava_mesh;
		}
	}

	vkCmdEndRendering(cmd);
	setupGBufferBarriers(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

LavaDeferredRenderSystem::~LavaDeferredRenderSystem()
{
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
		barrier.image = shadowmaps_[i]->get_allocated_image().image;
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

void LavaDeferredRenderSystem::allocateLights(std::vector<std::optional<struct LightComponent>>& light_component_vector)
{
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	//for each light we iterate over the every render component 
	for (; light_it != light_end; light_it++)
	{
		if (!light_it->has_value()) continue;

		LightComponent& light_component = light_it->value();
		if (light_component.allocated_) {
			if (light_component.type_ == light_component.allocated_type_) {
				continue;
			}
			else {
				engine_.global_descriptor_allocator_->freeDescriptorSet(light_component.descriptor_set_);
			}
		}

		engine_.global_descriptor_allocator_->clear();
		light_component.light_data_buffer_ = std::make_unique<LavaBuffer>(*engine_.allocator_, sizeof(LightShaderStruct), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
		light_component.light_data_buffer_->setMappedData();
		int viewproj_size = sizeof(glm::mat4);
		if (light_component.type_ == LIGHT_TYPE_DIRECTIONAL) viewproj_size *= 3; //Three layers in directional
		else if (light_component.type_ == LIGHT_TYPE_POINT) viewproj_size *= 6; //Six layers in point

		light_component.light_viewproj_buffer_ = std::make_unique<LavaBuffer>(*engine_.allocator_, viewproj_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
		light_component.light_viewproj_buffer_->setMappedData();
		light_component.descriptor_set_ = engine_.global_descriptor_allocator_->allocate(engine_.global_lights_descriptor_set_layout_);

		engine_.global_descriptor_allocator_->writeBuffer(0, light_component.light_data_buffer_->get_buffer().buffer, sizeof(LightShaderStruct), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		engine_.global_descriptor_allocator_->writeBuffer(1, light_component.light_viewproj_buffer_->get_buffer().buffer, viewproj_size, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		engine_.global_descriptor_allocator_->writeImage(2, shadowmaps_[2]->get_allocated_image().image_view,
			shadowmaps_[2]->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_->writeImage(3, shadowmaps_[1]->get_allocated_image().image_view,
			shadowmaps_[1]->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_->writeImage(4, shadowmaps_[0]->get_allocated_image().image_view,
			shadowmaps_[0]->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_->updateSet(light_component.descriptor_set_);
		engine_.global_descriptor_allocator_->clear();

		light_component.allocated_ = true;
		light_component.allocated_type_ = light_component.type_;
	}
}

void LavaDeferredRenderSystem::updateLights(std::vector<std::optional<struct LightComponent>>& light_component_vector, std::vector<std::optional<struct TransformComponent>>& transform_vector)
{
	auto light_transform_it = transform_vector.begin();
	auto light_transform_end = transform_vector.end();
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	//for each light we iterate over the every render component 
	for (; light_transform_it != light_transform_end || light_it != light_end; light_transform_it++, light_it++)
	{
		if (!light_transform_it->has_value()) continue;
		if (!light_it->has_value()) continue;

		if (!light_it->value().allocated_) continue;

		LightComponent& light_component = light_it->value();

		LightShaderStruct light_shader_struct = {};
		light_shader_struct.config(light_it->value(), light_transform_it->value());
		light_component.light_data_buffer_->updateBufferData(&light_shader_struct, sizeof(LightShaderStruct));


		//glm::vec3 forward = CalculateForwardVector(light_transform_it->value().rot_);


		if (light_component.type_ == LIGHT_TYPE_DIRECTIONAL) {
			glm::vec3 opp_light_dir = CalculateForwardVector(light_transform_it->value().rot_) * -1.0f;
			glm::vec3 pos = engine_.main_camera_transform_->pos_ + (opp_light_dir * 20.0f);

			glm::mat4 view = GenerateViewMatrix(
				pos,
				light_transform_it->value().rot_
			);

			float size = 5.0f; // Tamaño del área visible
			float left = -size;
			float right = size;
			float bottom = -size;
			float top = size;
			glm::mat4 proj = glm::ortho(left, right, bottom, top, 50.0f, 0.1f);
			proj[1][1] *= -1;
			light_component.viewproj_ = proj * view; // THE VIEW IS WRONG IN THIS TYPE OF LIGHT
			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.push_back(light_component.viewproj_);
			proj = glm::ortho(left * 2.0f, right * 2.0f, bottom * 2.0f, top * 2.0f, 50.0f, 0.1f);
			proj[1][1] *= -1;
			shadowTransforms.push_back(proj * view);
			proj = glm::ortho(left * 3.0f, right * 3.0f, bottom * 3.0f, top * 3.0f, 50.0f, 0.1f);
			proj[1][1] *= -1;
			shadowTransforms.push_back(proj * view);
			light_component.light_viewproj_buffer_->updateBufferData(shadowTransforms.data(), sizeof(glm::mat4) * 3);
		}
		else if (light_component.type_ == LIGHT_TYPE_SPOT) {
			glm::mat4 view = GenerateViewMatrix(
				light_transform_it->value().pos_,
				light_transform_it->value().rot_
			);

			float fov = 2.0f * light_component.cutoff_;

			float near = 10000.0f; // Plano cercano
			float far = 0.1f; // Plano lejano
			// Generar la matriz de proyección en perspectiva
			glm::mat4 proj = glm::perspective(glm::radians(fov), (float)shadowmaps_[2]->get_allocated_image().image_extent.width / (float)shadowmaps_[2]->get_allocated_image().image_extent.height, near, far);
			proj[1][1] *= -1;
			light_component.viewproj_ = proj * view;
			light_component.light_viewproj_buffer_->updateBufferData(&light_component.viewproj_, sizeof(glm::mat4));
		}
		else {
			glm::mat4 view = GenerateViewMatrix(
				light_transform_it->value().pos_,
				light_transform_it->value().rot_
			);

			float aspect = (float)shadowmaps_[0]->get_allocated_image().image_extent.width / (float)shadowmaps_[0]->get_allocated_image().image_extent.height;
			float near = 25.0f;//0.1f; // Deberian ser propiedades talvez?
			float far = 0.1f; //
			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
			std::vector<glm::mat4> shadowTransforms;
			glm::vec3 light_pos = light_transform_it->value().pos_;
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(light_pos, light_pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(light_pos, light_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(light_pos, light_pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(light_pos, light_pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(light_pos, light_pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
			shadowTransforms.push_back(shadowProj *
				glm::lookAt(light_pos, light_pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
			light_component.light_viewproj_buffer_->updateBufferData(shadowTransforms.data(), sizeof(glm::mat4) * 6);
		}
	}
}