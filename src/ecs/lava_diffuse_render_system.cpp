#include "lava/ecs/lava_diffuse_render_system.hpp"

#include "lava/engine/lava_engine.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_pipeline.hpp"
#include "lava/engine/lava_pbr_material.hpp"

LavaDiffuseRenderSystem::LavaDiffuseRenderSystem(LavaEngine &engine) :
  engine_{engine},
  pipeline_{ std::make_unique<LavaPipeline>(PipelineConfig(
							PIPELINE_TYPE_PBR,
              "../src/shaders/normal.vert.spv",
              "../src/shaders/diffuse.frag.spv",
              engine_.device_.get(),
              engine_.swap_chain_.get(),
              engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
              PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO))}
{

}


void LavaDiffuseRenderSystem::render(
  std::vector<std::optional<TransformComponent>>& transform_vector,
  std::vector<std::optional<RenderComponent>>& render_vector
  ) {
	


  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_->get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_->get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
	//
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(engine_.swap_chain_->get_draw_extent(), &color_attachment, &depth_attachment);
	vkCmdBeginRendering(engine_.commandBuffer, &renderInfo);

	engine_.setDynamicViewportAndScissor(engine_.swap_chain_->get_draw_extent());
	
	vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->get_pipeline());
	//Bind both descriptor sets on the mesh
	vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_->get_layout(),
		0, 1, &engine_.global_descriptor_set_, 0, nullptr);


	FrameData& frame_data = engine_.frame_data_->getCurrentFrame();
  //Draw everycomponent
  auto transform_it = transform_vector.begin();
  auto render_it = render_vector.begin();
  auto transform_end = transform_vector.end();
  auto render_end = render_vector.end();
  for (; transform_it != transform_end || render_it != render_end; transform_it++, render_it++) {
    if(!transform_it->has_value()) continue;
    if (!render_it->has_value()) continue;

		if (render_it->value().active_ != RenderType_UNLIT) continue;

		//Clean Descriptor sets for current frame
		frame_data.descriptor_manager.clear();

		std::shared_ptr<LavaMesh> lava_mesh = render_it->value().mesh_;
		std::shared_ptr<MeshAsset> mesh = lava_mesh->mesh_;

		VkDescriptorSet pbr_descriptor_set = lava_mesh->get_material()->get_descriptor_set();
		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_->get_layout(),
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
		
		vkCmdPushConstants(engine_.commandBuffer, pipeline_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		vkCmdDrawIndexed(engine_.commandBuffer, mesh->index_count, 1, 0, 0, 0);

		if (frame_data.last_bound_mesh != lava_mesh) {
			frame_data.last_bound_mesh = lava_mesh;
		}
  }

	vkCmdEndRendering(engine_.commandBuffer);

  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image,
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  //Cambiamos la imagen a tipo presentable para enseñarla en la superficie
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index],
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Devolvemos la imagen al swapchain
  CopyImageToImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image,
    engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_->get_draw_extent(), engine_.window_extent_);

}

LavaDiffuseRenderSystem::~LavaDiffuseRenderSystem()
{
}