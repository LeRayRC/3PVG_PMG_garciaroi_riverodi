#include "lava/ecs/lava_diffuse_render_system_vr.hpp"

#include "lava/vr/lava_engine_vr.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_pipeline.hpp"
#include "vr/lava_swapchain_vr.hpp"
#include "vr/lava_session_vr.hpp"
#include "lava/engine/lava_pbr_material.hpp"

LavaDiffuseRenderSystemVR::LavaDiffuseRenderSystemVR(LavaEngineVR &engine) :
  engine_{engine},
  pipeline_{ std::make_unique<LavaPipeline>(PipelineConfigVR(
							PIPELINE_TYPE_PBR,
              "../src/shaders/normal.vert.spv",
              "../src/shaders/diffuse.frag.spv",
              engine_.device_.get(),
              &engine_.get_swapchain(),
              engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
              PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET,
							PipelineBlendMode::PIPELINE_BLEND_ONE_ZERO))}
{

}


void LavaDiffuseRenderSystemVR::render(uint32_t view_index,
  std::vector<std::optional<TransformComponent>>& transform_vector,
  std::vector<std::optional<RenderComponent>>& render_vector
  ) {
	
	std::vector<LavaSwapchainVR::SwapchainInfo>& color_swapchain_info_vector = engine_.get_swapchain().get_color_swapchain_infos();
	std::vector<LavaSwapchainVR::SwapchainInfo>& depth_swapchain_info_vector = engine_.get_swapchain().get_depth_swapchain_infos();
	// Per view in the view configuration:
	//for (uint32_t i = 0; i < view_count_; i++) {
	LavaSwapchainVR::SwapchainInfo& color_swapchain_info = color_swapchain_info_vector[view_index];
	LavaSwapchainVR::SwapchainInfo& depth_swapchain_info = depth_swapchain_info_vector[view_index];
	VkImage color_image = engine_.get_swapchain().get_image_from_image_view(color_swapchain_info.imageViews[engine_.color_image_index_]);
	VkImage depth_image = engine_.get_swapchain().get_image_from_image_view(depth_swapchain_info.imageViews[engine_.depth_image_index_]);


  TransitionImage(engine_.command_buffer_, color_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  TransitionImage(engine_.command_buffer_, depth_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	//begin a render pass  connected to our draw image
	VkRenderingAttachmentInfo color_attachment = vkinit::AttachmentInfo((VkImageView)color_swapchain_info.imageViews[engine_.color_image_index_], nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment = vkinit::DepthAttachmentInfo((VkImageView)depth_swapchain_info.imageViews[engine_.depth_image_index_], VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
	//

	//engine_.get_session().get_view_configuration_views()[view_index].

	VkExtent2D window_extent = {
		engine_.get_session().get_view_configuration_views()[view_index].recommendedImageRectWidth,
		engine_.get_session().get_view_configuration_views()[view_index].recommendedImageRectHeight,
	};
	VkRenderingInfo renderInfo = vkinit::RenderingInfo(window_extent, &color_attachment, &depth_attachment);
	vkCmdBeginRendering(engine_.command_buffer_, &renderInfo);

	engine_.setDynamicViewportAndScissor(window_extent);
	
	vkCmdBindPipeline(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->get_pipeline());
	//Bind both descriptor sets on the mesh
	vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_->get_layout(),
		0, 1, &engine_.global_descriptor_set_vector_[view_index], 0, nullptr);


	FrameData& frame_data = engine_.frame_data_[view_index]->getCurrentFrame();
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
		vkCmdBindDescriptorSets(engine_.command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
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
		//if (frame_data.last_bound_mesh != lava_mesh) {
			vkCmdBindIndexBuffer(engine_.command_buffer_, meshBuffers.index_buffer->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
		//}
		
		push_constants.world_matrix = model; 
		push_constants.vertex_buffer = meshBuffers.vertex_buffer_address;
		
		vkCmdPushConstants(engine_.command_buffer_, pipeline_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
		vkCmdDrawIndexed(engine_.command_buffer_, mesh->index_count, 1, 0, 0, 0);

		if (frame_data.last_bound_mesh != lava_mesh) {
			frame_data.last_bound_mesh = lava_mesh;
		}
  }

	vkCmdEndRendering(engine_.command_buffer_);

  //TransitionImage(engine_.command_buffer_, color_image,
  //  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  ////Cambiamos la imagen a tipo presentable para enseñarla en la superficie
  //TransitionImage(engine_.command_buffer_, engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index],
  //  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  //// Devolvemos la imagen al swapchain
  //CopyImageToImage(engine_.command_buffer_, engine_.swap_chain_->get_draw_image().image,
  //  engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_->get_draw_extent(), engine_.window_extent_);

}

LavaDiffuseRenderSystemVR::~LavaDiffuseRenderSystemVR()
{
}