#include "lava/ecs/lava_pbr_render_system_vr.hpp"

#include "lava/vr/lava_engine_vr.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "engine/lava_descriptor_manager.hpp"
#include "engine/lava_allocator.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_pipeline.hpp"
#include "vr/lava_session_vr.hpp"
#include "vr/lava_swapchain_vr.hpp"
#include "lava/common/lava_global_helpers.hpp"

LavaPBRRenderSystemVR::LavaPBRRenderSystemVR(LavaEngineVR &engine) :
  engine_{engine},
	pipeline_{ std::make_unique<LavaPipeline>(PipelineConfigVR(
							PIPELINE_TYPE_PBR,
							"../src/shaders/vr/pbr.vert.spv",
							"../src/shaders/vr/pbr.frag.spv",
							engine_.device_.get(),
							engine_.swapchain_.get(),
							engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
							PipelineFlags::PIPELINE_USE_PUSHCONSTANTS
							| PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET
							,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ONE))},
	pipeline_first_light_{ std::make_unique<LavaPipeline>(PipelineConfigVR(
													PIPELINE_TYPE_PBR,
													"../src/shaders/vr/pbr.vert.spv",
													"../src/shaders/vr/pbr_ambient.frag.spv",
													engine_.device_.get(),
													engine_.swapchain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO))},
	pipeline_shadows_{ std::make_unique<LavaPipeline>(PipelineConfigVR( //TO DO: DIRECTIONAL LIGHT
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/shadow_mapping.vert.spv",
													"../src/shaders/shadow_mapping.frag.spv",
													engine_.device_.get(),
													engine_.swapchain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO/*,
													"../src/shaders/directional_shadows.geom.spv"*/)), 

					   std::make_unique<LavaPipeline>(PipelineConfigVR(
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/point_shadow_mapping.vert.spv",
													"../src/shaders/point_shadow_mapping.frag.spv",
													engine_.device_.get(),
													engine_.swapchain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO,
													"../src/shaders/point_shadows.geom.spv")), 

					   std::make_unique<LavaPipeline>(PipelineConfigVR(
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/shadow_mapping.vert.spv",
													"../src/shaders/shadow_mapping.frag.spv",
													engine_.device_.get(),
													engine_.swapchain_.get(),
													engine_.global_descriptor_allocator_.get(),
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO)), }

{
	VkExtent3D draw_image_extent = {
		2048,
		2048,
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
		else if(i == 1)layers = 6;

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


void LavaPBRRenderSystemVR::renderWithShadows(
	uint32_t view_index,
	std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector
) {

	allocate_lights(light_component_vector);
	update_lights(view_index,light_component_vector, transform_vector);

	FrameData& frame_data = engine_.frame_data_[view_index]->getCurrentFrame();
	int lights_rendered = 0;





	std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = engine_.get_swapchain().get_color_swapchain_infos();
	std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = engine_.get_swapchain().get_depth_swapchain_infos();
	LavaSwapchainVR::SwapchainInfo& color_swapchain_info = color_swapchain_info_vector[view_index];
	LavaSwapchainVR::SwapchainInfo& depth_swapchain_info = depth_swapchain_info_vector[view_index];
	VkImage color_image = engine_.get_swapchain().get_image_from_image_view(color_swapchain_info.imageViews[engine_.color_image_index_]);
	VkImage depth_image = engine_.get_swapchain().get_image_from_image_view(depth_swapchain_info.imageViews[engine_.depth_image_index_]);

	VkExtent2D window_extent = {
		engine_.get_session().get_view_configuration_views()[view_index].recommendedImageRectWidth,
		engine_.get_session().get_view_configuration_views()[view_index].recommendedImageRectHeight,
	};

	auto light_transform_it = transform_vector.begin();
	auto light_transform_end = transform_vector.end();
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	//for each light we iterate over Every render component to create the shadow map
	for (; light_transform_it != light_transform_end || light_it != light_end; light_transform_it++, light_it++)
	{
		if (!light_transform_it->has_value()) continue;
		if (!light_it->has_value()) continue;

		if (!light_it->value().enabled_) continue;

		int light_index = (int)light_it->value().type_;
		VkImage current_shadowmap = shadowmap_image_[light_index].image;;
		VkImageView current_shadowmap_image_view = shadowmap_image_[light_index].image_view;
		VkExtent2D current_extent;
		current_extent.width = shadowmap_image_[light_index].image_extent.width;
		current_extent.height = shadowmap_image_[light_index].image_extent.height;

		TransitionImage(engine_.command_buffer_, color_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		TransitionImage(engine_.command_buffer_, current_shadowmap, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);


		{
			VkClearValue clear_value;
			clear_value.color = { 0.0f,0.0f,0.0f,0.0f };
			VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo((VkImageView)color_swapchain_info.imageViews[engine_.color_image_index_], NULL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(current_shadowmap_image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR,1.0f);
			int layers = 1;
			if (light_index == 0) layers = 3;
			else if (light_index == 1)layers = 6;
			VkRenderingInfo renderInfo = vkinit::RenderingInfo(current_extent,nullptr, &depth_attachment, layers);
			vkCmdBeginRendering(engine_.command_buffer_, &renderInfo);
			engine_.setDynamicViewportAndScissor(current_extent);
		}

		//First Draw Create Shadow Map
		LavaPipeline* active_pipeline = pipeline_shadows_[light_index].get();
		vkCmdBindPipeline(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());

		vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			0, 1, &engine_.global_descriptor_set_vector_[view_index], 0, nullptr);

		vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
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

			//Clean Descriptor sets for current frame
			frame_data.descriptor_manager.clear();

			std::shared_ptr<LavaMesh> lava_mesh = render_it->value().mesh_;
			std::shared_ptr<MeshAsset> mesh = lava_mesh->mesh_;

			VkDescriptorSet pbr_descriptor_set = lava_mesh->get_material()->get_descriptor_set();
			vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
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
			//if (frame_data.last_bound_mesh != lava_mesh) {
				vkCmdBindIndexBuffer(engine_.command_buffer_, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
			//}

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
				vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
					active_pipeline->get_layout(),
					1, 1, &pbr_descriptor_set, 0, nullptr);

				// Configurar push constants
				GPUDrawPushConstants push_constants;
				push_constants.world_matrix = model;
				push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

				vkCmdPushConstants(engine_.command_buffer_, active_pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);

				// Dibujar la superficie
				vkCmdDrawIndexed(engine_.command_buffer_, surface.count, 1, surface.start_index, 0, 0);
			}

			//push_constants.world_matrix = model;
			//push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

			//vkCmdPushConstants(engine_.commandBuffer, active_pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
			//vkCmdDrawIndexed(engine_.commandBuffer, mesh->index_count, 1, 0, 0, 0);

			if (frame_data.last_bound_mesh != lava_mesh) {
				frame_data.last_bound_mesh = lava_mesh;
			}
		}

		vkCmdEndRendering(engine_.command_buffer_);

		TransitionImage(engine_.command_buffer_,color_image , VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		TransitionImage(engine_.command_buffer_, depth_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);


		for (int i = 0; i < 3; i++) {
			TransitionImage(engine_.command_buffer_,
				shadowmap_image_[i].image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, true);
		}

		{
			VkRenderingAttachmentInfo color_attachment;
			VkRenderingAttachmentInfo depth_attachment;
			if (lights_rendered < 1) {
				VkClearValue clear_value;
				clear_value.color = { 0.0f,0.0f,0.0f,0.0f };
				color_attachment = vkinit::AttachmentInfo((VkImageView)color_swapchain_info.imageViews[engine_.color_image_index_], &clear_value, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				depth_attachment = vkinit::DepthAttachmentInfo((VkImageView)depth_swapchain_info.imageViews[engine_.depth_image_index_], VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR,1.0f);
			}
			else {
				color_attachment = vkinit::AttachmentInfo((VkImageView)color_swapchain_info.imageViews[engine_.color_image_index_], NULL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				depth_attachment = vkinit::DepthAttachmentInfo((VkImageView)depth_swapchain_info.imageViews[engine_.depth_image_index_], VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD,1.0f);
			}
			VkRenderingInfo renderInfo = vkinit::RenderingInfo(window_extent, &color_attachment, &depth_attachment);
			vkCmdBeginRendering(engine_.command_buffer_, &renderInfo);
		}
		engine_.setDynamicViewportAndScissor(window_extent);


		if (lights_rendered >= 1) {
			active_pipeline = pipeline_.get();
		}
		else {
			active_pipeline = pipeline_first_light_.get();
		}

		vkCmdBindPipeline(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());

		vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			0, 1, &engine_.global_descriptor_set_vector_[view_index], 0, nullptr);

		vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			2, 1, &light_it->value().descriptor_set_, 0, nullptr);

		transform_it = transform_vector.begin();
		render_it = render_vector.begin();
		transform_end = transform_vector.end();
		render_end = render_vector.end();
		for (; transform_it != transform_end || render_it != render_end; transform_it++, render_it++) {
				if (!transform_it->has_value()) continue;
				if (!render_it->has_value()) continue;

				//Clean Descriptor sets for current frame
				//frame_data.descriptor_manager.clear();

		

				std::shared_ptr<LavaMesh> lava_mesh = render_it->value().mesh_;
				std::shared_ptr<MeshAsset> mesh = lava_mesh->mesh_;

				VkDescriptorSet pbr_descriptor_set = lava_mesh->get_material()->get_descriptor_set();
				vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
					active_pipeline->get_layout(),
					1, 1, &pbr_descriptor_set, 0, nullptr);

				GPUDrawPushConstants push_constants;
				glm::mat4 model = glm::mat4(1.0f);

				model = glm::translate(model, transform_it->value().pos_);
				model = glm::rotate(model, glm::radians(transform_it->value().rot_.x), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(transform_it->value().rot_.y), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(transform_it->value().rot_.z), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, transform_it->value().scale_);


				glm::vec4 pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				glm::vec4 fragPosLightSpace = light_it->value().viewproj_ * model * pos;
				glm::vec4 projCoords = fragPosLightSpace / fragPosLightSpace.w;

				// Vincular los Vertex y Index Buffers
				GPUMeshBuffers& meshBuffers = mesh->meshBuffers;
				VkDeviceSize offsets[] = { 0 };
				//if (frame_data.last_bound_mesh != lava_mesh) {
					vkCmdBindIndexBuffer(engine_.command_buffer_, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
				//}

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
					vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
						active_pipeline->get_layout(),
						1, 1, &pbr_descriptor_set, 0, nullptr);

					// Configurar push constants
					GPUDrawPushConstants push_constants;
					push_constants.world_matrix = model;
					push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

					vkCmdPushConstants(engine_.command_buffer_, active_pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);

					// Dibujar la superficie
					vkCmdDrawIndexed(engine_.command_buffer_, surface.count, 1, surface.start_index, 0, 0);
				}

				//push_constants.world_matrix = model;
				//push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

				//vkCmdPushConstants(engine_.commandBuffer, active_pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
				//vkCmdDrawIndexed(engine_.commandBuffer, mesh->index_count, 1, 0, 0, 0);

				if (frame_data.last_bound_mesh != lava_mesh) {
					frame_data.last_bound_mesh = lava_mesh;
				}
			}

		lights_rendered++;

		vkCmdEndRendering(engine_.command_buffer_);

	}
}

/*	******************************** NEED FIX: ************************************
	1- Should not be call each frame
	2- Should be call each time a light change certains properties(ex: light_type)
*/
void LavaPBRRenderSystemVR::allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector) 
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

		engine_.global_descriptor_allocator_->writeImage(2, shadowmap_image_[2].image_view,
			shadowmap_sampler_[2],
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_->writeImage(3, shadowmap_image_[1].image_view,
			shadowmap_sampler_[1],
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_->writeImage(4, shadowmap_image_[0].image_view,
			shadowmap_sampler_[0],
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_->updateSet(light_component.descriptor_set_);
		engine_.global_descriptor_allocator_->clear();

		light_component.allocated_ = true;
		light_component.allocated_type_ = light_component.type_;
	}
}

static std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	const auto inv = glm::inverse(proj * view);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt =
					inv * glm::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

void LavaPBRRenderSystemVR::update_lights(
	uint32_t view_index,
	std::vector<std::optional<struct LightComponent>>& light_component_vector, std::vector<std::optional<struct TransformComponent>>& transform_vector)
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


		if (light_component.type_ == LIGHT_TYPE_DIRECTIONAL) {

				std::vector<glm::vec4> corners = getFrustumCornersWorldSpace(engine_.global_scene_data_vector_[view_index].proj, engine_.main_camera_camera_->view_);

				glm::vec3 center = glm::vec3(0, 0, 0);
				for (const glm::vec4& v : corners)
				{
					center += glm::vec3(v);
				}
				center /= corners.size();
				printf("%0.2f,%0.2f,%0.2f\n", center.x, center.y, center.z);
				
				glm::vec3 forward = CalculateForwardVector(light_transform_it->value().rot_);
				glm::vec3 pos = center - 10.0f * forward;

				glm::mat4 light_view = GenerateViewMatrix(pos, light_transform_it->value().rot_);

			// Generar la matriz de proyección en perspectiva
			float size = 10.0f;
			float left = -size;
			float right = size;
			float bottom = -size;
			float top = size;
			glm::mat4 proj = glm::ortho(left, right, bottom, top, engine_.main_camera_camera_->near_, engine_.main_camera_camera_->far_);


			proj[1][1] *= -1;
			light_component.viewproj_ = proj * light_view;
			light_component.light_viewproj_buffer_->updateBufferData(&light_component.viewproj_, sizeof(glm::mat4));

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
			glm::mat4 proj = glm::perspective(glm::radians(fov), (float)shadowmap_image_[2].image_extent.width / (float)shadowmap_image_[2].image_extent.height, near, far);
			proj[1][1] *= -1;
			light_component.viewproj_ = proj * view;
			light_component.light_viewproj_buffer_->updateBufferData(&light_component.viewproj_, sizeof(glm::mat4));
		}
		else {
			glm::mat4 view = GenerateViewMatrix(
				light_transform_it->value().pos_,
				light_transform_it->value().rot_
			);

			float aspect = (float)shadowmap_image_[0].image_extent.width / (float)shadowmap_image_[0].image_extent.height;
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

LavaPBRRenderSystemVR::~LavaPBRRenderSystemVR()
{
	for (int i = 0; i < 3; i++) {
		vkDestroySampler(engine_.device_->get_device(), shadowmap_sampler_[i], nullptr);
		vkDestroyImageView(engine_.device_->get_device(), shadowmap_image_[i].image_view, nullptr);
		vmaDestroyImage(engine_.allocator_->get_allocator(), shadowmap_image_[i].image, shadowmap_image_[i].allocation);
	}
}