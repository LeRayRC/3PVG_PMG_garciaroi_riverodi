#include "lava/ecs/lava_gaussian_render_system.hpp"

#include "lava/engine/lava_engine.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "engine/lava_frame_data.hpp"
#include "engine/lava_pipeline.hpp"
#include "lava/gaussian/lava_gaussian_splat.hpp"
#include "lava/common/lava_global_helpers.hpp"

static int32_t calcOrder(const glm::vec3& dir)
{
	//{0, 0, 0}    //13 //ERROR
	const int32_t ax = (int32_t)roundf(dir.x + 1.0f);
	const int32_t ay = (int32_t)roundf(dir.y + 1.0f);
	const int32_t az = (int32_t)roundf(dir.z + 1.0f);

	int32_t index = (ax * 9) + (ay * 3) + az;
	return (index < 13 ? index : index - 1);
}

LavaGaussianRenderSystem::LavaGaussianRenderSystem(LavaEngine &engine) : 
  engine_{engine},
  pipeline_{ std::make_unique<LavaPipeline>(PipelineConfigGS(
							PIPELINE_TYPE_PBR,
              "../src/shaders/gaussian/gsplat.vert.spv",
              "../src/shaders/gaussian/gsplat.frag.spv",
              engine_.device_.get(),
              engine_.swap_chain_.get(),
              engine_.global_descriptor_allocator_.get(),
							engine_.global_descriptor_set_layout_,
							engine_.global_pbr_descriptor_set_layout_,
							engine_.global_lights_descriptor_set_layout_,
              PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET
			  ,"../src/shaders/gaussian/gsplat.geom.spv"
							))}
{

}


void LavaGaussianRenderSystem::render(
  TransformComponent& transform,
  LavaGaussianSplat& gaussian
  ) {
	


  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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

	//Clean Descriptor sets for current frame
	frame_data.descriptor_manager.clear();


	GPUGSDrawPushConstants push_constants;
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, transform.pos_);
	model = glm::rotate(model, glm::radians(transform.rot_.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(transform.rot_.y + 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(transform.rot_.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, transform.scale_);

	glm::vec3 fwd = CalculateForwardVector(engine_.main_camera_transform_->rot_);
	fwd *= -1.0f;
	int ind = calcOrder(fwd);
	vkCmdBindIndexBuffer(engine_.commandBuffer, gaussian.index_buffer[ind]->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
	
	push_constants.world_matrix = model; 
	push_constants.vertex_buffer = gaussian.vertex_buffer_address;
	push_constants.gsData = glm::vec4(engine_.main_camera_camera_->near_, engine_.main_camera_camera_->far_,
									  (float)engine_.window_extent_.width, (float)engine_.window_extent_.height);
	
	vkCmdPushConstants(engine_.commandBuffer, pipeline_->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUGSDrawPushConstants), &push_constants);
	vkCmdDrawIndexed(engine_.commandBuffer, (uint32_t)gaussian.getNumGaussians(), 1, 0, 0, 0);

	vkCmdEndRendering(engine_.commandBuffer);

  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image,
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  //Cambiamos la imagen a tipo presentable para ense�arla en la superficie
  TransitionImage(engine_.commandBuffer, engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index],
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Devolvemos la imagen al swapchain
  CopyImageToImage(engine_.commandBuffer, engine_.swap_chain_->get_draw_image().image,
    engine_.swap_chain_->get_swap_chain_images()[engine_.swap_chain_image_index], engine_.swap_chain_->get_draw_extent(), engine_.window_extent_);

}

LavaGaussianRenderSystem::~LavaGaussianRenderSystem()
{
}