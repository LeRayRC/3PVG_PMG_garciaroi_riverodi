#include "engine/lava_pipeline.hpp"

#include "engine/lava_pipeline_builder.hpp"
#include "lava_types.hpp"
#include "lava_vulkan_helpers.hpp"
#include "engine/lava_descriptor_manager.hpp"


LavaPipeline::LavaPipeline(PipelineConfig config){

	device_ = config.device->get_device();

	VkShaderModule vertex_shader;
	if (!LoadShader(config.vertex_shader_path, device_, &vertex_shader)) {
		printf("Fragment shader loader failed\n");
		exit(-1);
	}

	VkShaderModule fragment_shader;
	if (!LoadShader(config.fragment_shader_path, device_, &fragment_shader)) {
		printf("Vertex shader loader failed\n");
		exit(-1);
	}

	VkPushConstantRange buffer_range{};
	buffer_range.offset = 0;
	buffer_range.size = sizeof(GPUDrawPushConstants);
	buffer_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pNext = nullptr;

	//Create Generic pipeline layouts 10 descriptor sets with two textures each 
	
	if ((config.pipeline_flags & PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET) != 0) {
		configureDescriptorSet(&pipeline_layout_info, 
			config.descriptor_set_layout,
			config.type,
			config.descriptor_manager);
	}
	else {
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = VK_NULL_HANDLE;
	}

	if ((config.pipeline_flags & PipelineFlags::PIPELINE_USE_PUSHCONSTANTS) != 0) {
		configurePushConstants(&pipeline_layout_info, &buffer_range);
	}

	//pipeline_layout_info.pPushConstantRanges = &buffer_range;
	//pipeline_layout_info.pushConstantRangeCount = 1;

	vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &layout_);
	//assert(layout_ != nullptr, "Compute pipeline creation failed!");
	

	PipelineBuilder pipeline_builder;

	//use the triangle layout we created
	pipeline_builder._pipeline_layout = layout_;
	//connecting the vertex and pixel shaders to the pipeline
	pipeline_builder.SetShaders(vertex_shader, fragment_shader);
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
	//pipeline_builder.DisableDepthtest();
	pipeline_builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	//connect the image format we will draw into, from draw image
	pipeline_builder.SetColorAttachmentFormat(config.swap_chain->get_draw_image().image_format);
	pipeline_builder.SetDepthFormat(config.swap_chain->get_depth_image().image_format);

	//finally build the pipeline
	pipeline_ = pipeline_builder.BuildPipeline(device_);

	//clean structures
	vkDestroyShaderModule(device_, fragment_shader, nullptr);
	vkDestroyShaderModule(device_, vertex_shader, nullptr);

	//Create descriptor set that will be destroyed when the engine gets stopped
	//Allocate descriptor set


}

LavaPipeline::~LavaPipeline() {
	vkDeviceWaitIdle(device_);
	vkDestroyPipeline(device_, pipeline_, nullptr);
	vkDestroyPipelineLayout(device_, layout_, nullptr);
	//First descriptor set is global and it is destroyed by the engine
	vkDestroyDescriptorSetLayout(device_, descriptor_set_layouts_[1], nullptr);
	
}

void LavaPipeline::configurePushConstants(VkPipelineLayoutCreateInfo* info,
	VkPushConstantRange* range) {
	info->pPushConstantRanges = range;
	info->pushConstantRangeCount = 1;
}


void LavaPipeline::configureDescriptorSet(VkPipelineLayoutCreateInfo* info, 
	VkDescriptorSetLayout global_layout, 
	PipelineType type, 
	class LavaDescriptorManager* descriptor_manager) {
	//Include global descriptor set also
	//Primero le decimos que va a utilizar un descriptor set global que tiene el motor
	//Este descriptor contiene cosas como la matriz de vista
	descriptor_set_layouts_[0] = global_layout;

	//By default every material can hold two images: diffuse and normal
	//TODO include Uniform buffer as another binding
	DescriptorLayoutBuilder builder;

	//En funcion del tipo de pipeline se configuran unos binding u otros

	switch (type)
	{
	case PIPELINE_TYPE_NORMAL: {
			builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
			builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);


			break;
		}
	case PIPELINE_TYPE_PBR: {
			/*
					float metallic_factor_;	//Determines the metallic value of the metallic parts of the texture
					float roughness_factor_;	//Determines the roughness value of the metallic parts of the texture
																		// Default value 0.5f
					float specular_factor_; 
					float opacity_mask_;
					float use_normal_;	//Determines to use 
			*/
			builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // base color texture
			builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // normal
			builder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // metallic_texture
			builder.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // roughness texture
			builder.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // opacity


			//builder.addBinding(3, );
			break;
		}
	default:
		break;
	}
	

	descriptor_set_layouts_[1] = builder.build(device_, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
	

	info->pSetLayouts = descriptor_set_layouts_;
	info->setLayoutCount = 2;


	descriptor_set_ = descriptor_manager->allocate(descriptor_set_layouts_[1]);

}
void LavaPipeline::configureAttributes(VkPipelineLayoutCreateInfo* info) {
	//To implement
}
