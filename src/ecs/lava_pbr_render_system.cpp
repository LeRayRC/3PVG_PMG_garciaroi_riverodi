#include "ecs/lava_pbr_render_system.hpp"

#include "lava_engine.hpp"
#include "lava_vulkan_inits.hpp"
#include "engine/lava_mesh.hpp"
#include "engine/lava_pbr_material.hpp"
LavaPBRRenderSystem::LavaPBRRenderSystem(LavaEngine &engine) :
  engine_{engine},
	pipeline_{ PipelineConfig(
							PIPELINE_TYPE_PBR,
							"../src/shaders/pbr.vert.spv",
							"../src/shaders/pbr.frag.spv",
							&engine_.device_,
							&engine_.swap_chain_,
							&engine_.global_descriptor_allocator_,
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
							PipelineFlags::PIPELINE_USE_PUSHCONSTANTS
							| PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET
							,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ONE)},
	pipeline_first_light_{ PipelineConfig(
													PIPELINE_TYPE_PBR,
													"../src/shaders/pbr.vert.spv",
													"../src/shaders/pbr_ambient.frag.spv",
													&engine_.device_,
													&engine_.swap_chain_,
													&engine_.global_descriptor_allocator_,
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO)},
	pipeline_shadows_{ PipelineConfig(
													PIPELINE_TYPE_SHADOW,
													"../src/shaders/shadow_mapping.vert.spv",
													"../src/shaders/shadow_mapping.frag.spv",
													&engine_.device_,
													&engine_.swap_chain_,
													&engine_.global_descriptor_allocator_,
													engine_.global_descriptor_set_layout_,
													engine_.global_pbr_descriptor_set_layout_,
													engine_.global_lights_descriptor_set_layout_,
													PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET | PipelineFlags::PIPELINE_DONT_USE_COLOR_ATTACHMENT,
													PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO) }

{
	VkExtent3D draw_image_extent = {
		1024,
		1024,
		1
	};

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//Create shadow map on the swapchain
	shadowmap_image_.image_format = VK_FORMAT_D32_SFLOAT;
	shadowmap_image_.image_extent = draw_image_extent;
	VkImageUsageFlags shadowmap_image_usages{};
	shadowmap_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	shadowmap_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo shadowmap_img_info = vkinit::ImageCreateInfo(shadowmap_image_.image_format,
		shadowmap_image_usages, draw_image_extent);

	//allocate and create the image
	vmaCreateImage(engine.allocator_.get_allocator(), &shadowmap_img_info, &rimg_allocinfo, &shadowmap_image_.image, &shadowmap_image_.allocation, nullptr);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo shadowmap_view_info = vkinit::ImageViewCreateInfo(shadowmap_image_.image_format, shadowmap_image_.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	if (vkCreateImageView(engine.device_.get_device(), &shadowmap_view_info, nullptr, &shadowmap_image_.image_view) !=
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


	vkCreateSampler(engine.device_.get_device(), &sampler_info, nullptr, &shadowmap_sampler_);


}


void LavaPBRRenderSystem::render(
  std::vector<std::optional<TransformComponent>>& transform_vector,
  std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector
  ) {
	
	int lights_rendered = 0;
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkClearValue clear_value;
	clear_value.color = { 0.0f,0.0f,0.0f,0.0f };
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_.get_draw_image().image_view, &clear_value, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_.get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(engine_.swap_chain_.get_draw_extent(), &color_attachment, &depth_attachment);
	vkCmdBeginRendering(engine_.commandBuffer, &renderInfo);

	//set dynamic viewport and scissor
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)engine_.swap_chain_.get_draw_extent().width;
	viewport.height = (float)engine_.swap_chain_.get_draw_extent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(engine_.commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = engine_.swap_chain_.get_draw_extent().width;
	scissor.extent.height = engine_.swap_chain_.get_draw_extent().height;
	vkCmdSetScissor(engine_.commandBuffer, 0, 1, &scissor);


	FrameData& frame_data = engine_.frame_data_.getCurrentFrame();
  //Draw everycomponent

	//First draw with ambient only
	LavaPipeline* active_pipeline = &pipeline_first_light_;
	vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());


	auto light_transform_it = transform_vector.begin();
	auto light_transform_end = transform_vector.end();
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	//for each light we iterate over the every render component 
	for (; light_transform_it != light_transform_end || light_it != light_end; light_transform_it++, light_it++)
	{
		if (!light_transform_it->has_value()) continue;
		if (!light_it->has_value()) continue;

		if (!light_it->value().enabled_) continue;

		if (lights_rendered == 1) {
			active_pipeline = &pipeline_;
			vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());
		}

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
			if(!transform_it->has_value()) continue;
			if (!render_it->has_value()) continue;

			//Clean Descriptor sets for current frame
			frame_data.descriptor_manager.clear();

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

		lights_rendered++;


	}

	vkCmdEndRendering(engine_.commandBuffer);

  TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image,
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  //Cambiamos la imagen a tipo presentable para enseñarla en la superficie
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_swap_chain_images()[engine_.swap_chain_image_index],
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Devolvemos la imagen al swapchain
  CopyImageToImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image,
    engine_.swap_chain_.get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_.get_draw_extent(), engine_.window_extent_);

}


void LavaPBRRenderSystem::renderWithShadows(
	std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<RenderComponent>>& render_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector
) {

	allocate_lights(light_component_vector);
	update_lights(light_component_vector, transform_vector);

	FrameData& frame_data = engine_.frame_data_.getCurrentFrame();
	int lights_rendered = 0;



	//auto transform_it = transform_vector.begin();
	//auto render_it = render_vector.begin();
	//auto transform_end = transform_vector.end();
	//auto render_end = render_vector.end();

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

		VkImage current_shadowmap;
		VkImageView current_shadowmap_image_view;
		VkExtent2D current_extent;
		if (light_it->value().type_ == LightType::LIGHT_TYPE_SPOT) {
			current_shadowmap = shadowmap_image_.image;
			current_shadowmap_image_view = shadowmap_image_.image_view;
			current_extent.width = shadowmap_image_.image_extent.width;
			current_extent.height = shadowmap_image_.image_extent.height;
		}
		else if (light_it->value().type_ == LightType::LIGHT_TYPE_POINT){

		}

		TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		TransitionImage(engine_.commandBuffer, current_shadowmap, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);


		{
			VkClearValue clear_value;
			clear_value.color = { 0.0f,0.0f,0.0f,0.0f };
			VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_.get_draw_image().image_view, NULL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(current_shadowmap_image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
			VkRenderingInfo renderInfo = vkinit::RenderingInfo(current_extent,nullptr, &depth_attachment);
			vkCmdBeginRendering(engine_.commandBuffer, &renderInfo);
			engine_.setDynamicViewportAndScissor(current_extent);
		}

		//First Draw Create Shadow Map
		LavaPipeline* active_pipeline = &pipeline_shadows_;
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

			//Clean Descriptor sets for current frame
			frame_data.descriptor_manager.clear();

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

		TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		//TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_shadowmap_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		TransitionImage(engine_.commandBuffer,
			current_shadowmap,
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, true);


		{
			VkRenderingAttachmentInfo color_attachment;
			VkRenderingAttachmentInfo depth_attachment;
			if (lights_rendered < 1) {
				VkClearValue clear_value;
				clear_value.color = { 1.0f,1.0f,1.0f,1.0f };
				color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_.get_draw_image().image_view, &clear_value, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_.get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
			}
			else {
				color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_.get_draw_image().image_view, NULL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_.get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
			}
			VkRenderingInfo renderInfo = vkinit::RenderingInfo(engine_.swap_chain_.get_draw_extent(), &color_attachment, &depth_attachment);
			vkCmdBeginRendering(engine_.commandBuffer, &renderInfo);
		}
		engine_.setDynamicViewportAndScissor(engine_.swap_chain_.get_draw_extent());


		if (lights_rendered >= 1) {
			active_pipeline = &pipeline_;
		}
		else {
			active_pipeline = &pipeline_first_light_;
		}
		vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_pipeline->get_pipeline());

		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			active_pipeline->get_layout(),
			0, 1, &engine_.global_descriptor_set_, 0, nullptr);

		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
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
				frame_data.descriptor_manager.clear();

		

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


				glm::vec4 pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				glm::vec4 fragPosLightSpace = light_it->value().viewproj_ * model * pos;
				glm::vec4 projCoords = fragPosLightSpace / fragPosLightSpace.w;

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

		lights_rendered++;

		vkCmdEndRendering(engine_.commandBuffer);

	}


	TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//Cambiamos la imagen a tipo presentable para enseñarla en la superficie
	TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_swap_chain_images()[engine_.swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Devolvemos la imagen al swapchain
	CopyImageToImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image,
		engine_.swap_chain_.get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_.get_draw_extent(), engine_.window_extent_);

}

void LavaPBRRenderSystem::allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector)
{
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();
	//for each light we iterate over the every render component 
	for (; light_it != light_end; light_it++)
	{
		if (!light_it->has_value()) continue;

		if (light_it->value().allocated_) continue;

		LightComponent& light_component = light_it->value();
		engine_.global_descriptor_allocator_.clear();
		light_component.light_data_buffer_ = std::make_unique<LavaBuffer>(engine_.allocator_, sizeof(LightShaderStruct), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
		light_component.light_data_buffer_->setMappedData();
		light_component.light_viewproj_buffer_ = std::make_unique<LavaBuffer>(engine_.allocator_, sizeof(LightShaderStruct), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
		light_component.light_viewproj_buffer_->setMappedData();

		light_component.descriptor_set_ = engine_.global_descriptor_allocator_.allocate(engine_.global_lights_descriptor_set_layout_);
		engine_.global_descriptor_allocator_.writeBuffer(0, light_component.light_data_buffer_->get_buffer().buffer, sizeof(LightShaderStruct), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		engine_.global_descriptor_allocator_.writeBuffer(1, light_component.light_viewproj_buffer_->get_buffer().buffer, sizeof(glm::mat4), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

		engine_.global_descriptor_allocator_.writeImage(2, shadowmap_image_.image_view,
			shadowmap_sampler_,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		engine_.global_descriptor_allocator_.updateSet(light_component.descriptor_set_);
		engine_.global_descriptor_allocator_.clear();



		light_component.allocated_ = true;
	}
}

void LavaPBRRenderSystem::update_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector, std::vector<std::optional<struct TransformComponent>>& transform_vector)
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
		glm::mat4 view = GenerateViewMatrix(
			light_transform_it->value().pos_,
			light_transform_it->value().rot_
		);

		if (light_component.type_ == LIGHT_TYPE_DIRECTIONAL) {
			float size = 1.0f; // Tamaño del área visible
			float left = -size;
			float right = size;
			float bottom = -size;
			float top = size;
			glm::mat4 proj = glm::ortho(left, right, bottom, top, 5.0f, 0.1f);
			proj[1][1] *= -1;
			light_component.viewproj_ = proj * view;
		}
		else if (light_component.type_ == LIGHT_TYPE_SPOT) {
			float fov = 2.0f * light_component.cutoff_;

			float near = 10000.0f; // Plano cercano
			float far = 0.1f; // Plano lejano
			// Generar la matriz de proyección en perspectiva
			glm::mat4 proj = glm::perspective(glm::radians(fov), (float)shadowmap_image_.image_extent.width / (float)shadowmap_image_.image_extent.height, near, far);
			proj[1][1] *= -1;
			light_component.viewproj_ = proj * view;
		}

		light_component.light_viewproj_buffer_->updateBufferData(&light_component.viewproj_, sizeof(glm::mat4));

	}
}

LavaPBRRenderSystem::~LavaPBRRenderSystem()
{
	vkDestroySampler(engine_.device_.get_device(), shadowmap_sampler_, nullptr);
	vkDestroyImageView(engine_.device_.get_device(), shadowmap_image_.image_view, nullptr);
	vmaDestroyImage(engine_.allocator_.get_allocator(), shadowmap_image_.image, shadowmap_image_.allocation);
}