#include "engine/lava_pipeline.hpp"

#include "engine/lava_pipeline_builder.hpp"
#include "lava_types.hpp"
#include "lava_vulkan_helpers.hpp"


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
	
	if ((config.pipeline_flags & PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET) != 0) {
		configureDescriptorSet(&pipeline_layout_info, config.descriptor_set_layout);
	}
	else {
		pipeline_layout_info.setLayoutCount = 0;
	}

	if ((config.pipeline_flags & PipelineFlags::PIPELINE_USE_PUSHCONSTANTS) != 0) {
		configurePushConstants(&pipeline_layout_info, &buffer_range);
	}

	//pipeline_layout_info.pPushConstantRanges = &buffer_range;
	//pipeline_layout_info.pushConstantRangeCount = 1;

	vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &layout_);
	assert(layout_ != nullptr, "Compute pipeline creation failed!");
	

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
}

LavaPipeline::~LavaPipeline() {
	vkDeviceWaitIdle(device_);
	vkDestroyPipeline(device_, pipeline_, nullptr);
	vkDestroyPipelineLayout(device_, layout_, nullptr);
}

void LavaPipeline::configurePushConstants(VkPipelineLayoutCreateInfo* info,
	VkPushConstantRange* range) {
	info->pPushConstantRanges = range;
	info->pushConstantRangeCount = 1;
}
void LavaPipeline::configureDescriptorSet(VkPipelineLayoutCreateInfo* info, VkDescriptorSetLayout layout) {
	info->pSetLayouts = &layout;
	info->setLayoutCount = 1;
}
void LavaPipeline::configureAttributes(VkPipelineLayoutCreateInfo* info) {
	//To implement
}
