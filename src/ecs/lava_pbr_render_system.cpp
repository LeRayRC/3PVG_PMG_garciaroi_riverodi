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
              PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET) }
{


}


void LavaPBRRenderSystem::render(
  std::vector<std::optional<TransformComponent>>& transform_vector,
  std::vector<std::optional<RenderComponent>>& render_vector
  ) {
	


  TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_.get_depth_image().image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo(engine_.swap_chain_.get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo(engine_.swap_chain_.get_depth_image().image_view, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	//
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


	vkCmdBindPipeline(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.get_pipeline());
	//Bind both descriptor sets on the mesh
	vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_.get_layout(),
		0, 1, &engine_.global_descriptor_set_, 0, nullptr);


	FrameData& frame_data = engine_.frame_data_.getCurrentFrame();
  //Draw everycomponent
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

		GPUDrawPushConstants push_constants;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, transform_it->value().pos_);
		model = glm::rotate(model, glm::radians(transform_it->value().rot_.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(transform_it->value().rot_.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(transform_it->value().rot_.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, transform_it->value().scale_);

		VkDescriptorSet pbr_descriptor_set = pipeline_.get_descriptor_set();

		//If material changes then the images are updated from descriptor set
		//if()
		engine_.global_descriptor_allocator_.clear();
		engine_.global_descriptor_allocator_.writeImage(
			0,
			lava_mesh->get_material()->base_color_->get_allocated_image().image_view,
			lava_mesh->get_material()->base_color_->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		
		engine_.global_descriptor_allocator_.updateSet(pbr_descriptor_set);
		engine_.global_descriptor_allocator_.clear();


		vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
														pipeline_.get_layout(),
														1, 1, &pbr_descriptor_set, 0, nullptr);


		// Vincular los Vertex y Index Buffers
		GPUMeshBuffers& meshBuffers = mesh->meshBuffers;
		VkDeviceSize offsets[] = { 0 };
		//VkBuffer vertex_buffer = meshBuffers.vertex_buffer->get_buffer().buffer;
		//vkCmdBindVertexBuffers(engine_.commandBuffer, 0, 1, &vertex_buffer, offsets);
		if (frame_data.last_bound_mesh != lava_mesh) {
			vkCmdBindIndexBuffer(engine_.commandBuffer, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		}
		
		push_constants.world_matrix = model; // global_scene_data_.viewproj* model;
		push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;

		// Dibujar cada superficie
		//int count_surfaces = mesh->count_surfaces;
		//int total_count = 0;
		//for (int i = 0; i < count_surfaces;i++) {
		//	GeoSurface& surface = mesh->surfaces[i];
		//	total_count += surface.count;
		//}
		
		vkCmdPushConstants(engine_.commandBuffer, pipeline_.get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		vkCmdDrawIndexed(engine_.commandBuffer, mesh->index_count, 1, 0, 0, 0);

		if (frame_data.last_bound_mesh != lava_mesh) {
			frame_data.last_bound_mesh = lava_mesh;
		}


		//VkDescriptorSet image_set = mesh->get_material()->get_descriptor_set();
		//vkCmdBindDescriptorSets(engine_.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		//	pipeline_.get_layout(),
		//	1, 1, &image_set, 0, nullptr);

		//push_constants.world_matrix = model; // global_scene_data_.viewproj* model;
		//
		//for (std::shared_ptr<MeshAsset> submesh : mesh->meshes_) {
		//	push_constants.vertex_buffer = submesh->meshBuffers.vertex_buffer_address;
		//	vkCmdBindIndexBuffer(engine_.commandBuffer, submesh->meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		//	vkCmdPushConstants(engine_.commandBuffer, pipeline_.get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		//	vkCmdDrawIndexed(engine_.commandBuffer, submesh->surfaces[0].count, 1, submesh->surfaces[0].start_index, 0, 0);
		//	
		//}
		//last_mesh = mesh;

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

LavaPBRRenderSystem::~LavaPBRRenderSystem()
{
}