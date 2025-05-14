#ifndef __LAVA_PIPELINE_H_
#define __LAVA_PIPELINE_H_ 1

#include "engine/lava_device.hpp"
#include "engine/lava_swap_chain.hpp"
#include "lava/common/lava_types.hpp"

typedef enum PipelineType {
	PIPELINE_TYPE_NORMAL,
	PIPELINE_TYPE_PBR,
	PIPELINE_TYPE_SHADOW
} PipelineType;

typedef enum PipelineFlags {
	PIPELINE_USE_ATTRIBUTES = 1,					//0001
	PIPELINE_USE_PUSHCONSTANTS = 1 << 1,	//0010
	PIPELINE_USE_DESCRIPTOR_SET = 1 << 2,  //0100
	PIPELINE_DONT_USE_COLOR_ATTACHMENT = 1 << 3, //1000
} PipelineFlags;

typedef enum PipelineBlendMode {
	PIPELINE_BLEND_DISABLE,
	PIPELINE_BLEND_ONE_ONE,
	PIPELINE_BLEND_ONE_ZERO
} PipelineBlendMode;

struct PipelineConfig {
public:
	PipelineType type;
	const char* vertex_shader_path;
	const char* fragment_shader_path;
	class LavaDevice* device;
	class LavaSwapChain* swap_chain;
	class LavaDescriptorManager* descriptor_manager;
	VkDescriptorSetLayout global_descriptor_set_layout;
	VkDescriptorSetLayout global_pbr_descriptor_set_layout;
	VkDescriptorSetLayout global_lights_descriptor_set_layout;
	int pipeline_flags;
	PipelineBlendMode blend_mode;
	const char* geometry_shader_path = nullptr;
	int color_attachments_count = 1;
	VkCompareOp compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL;
};

struct PipelineConfigVR {
public:
	PipelineType type;
	const char* vertex_shader_path;
	const char* fragment_shader_path;
	class LavaDevice* device;
	class LavaSwapchainVR* swap_chain;
	class LavaDescriptorManager* descriptor_manager;
	VkDescriptorSetLayout global_descriptor_set_layout;
	VkDescriptorSetLayout global_pbr_descriptor_set_layout;
	VkDescriptorSetLayout global_lights_descriptor_set_layout;
	int pipeline_flags;
	PipelineBlendMode blend_mode;
	const char* geometry_shader_path = nullptr;
	int color_attachments_count = 1;
	VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
};

class LavaPipeline
{
public:
	LavaPipeline(PipelineConfigVR config);
	LavaPipeline(PipelineConfig config);
	~LavaPipeline();

	VkPipelineLayout get_layout() { return layout_;}
	VkPipeline get_pipeline() { return pipeline_; }
	VkDescriptorSetLayout* get_descriptor_set_layouts() {
		return descriptor_set_layouts_;
	}

	VkDescriptorSet get_descriptor_set() {
		return descriptor_set_;
	}
	


private:
	int flags_;
	VkPipelineLayout layout_;
	VkPipeline pipeline_;
	VkDevice device_;
	VkDescriptorSetLayout descriptor_set_layouts_[3];
	VkDescriptorSet descriptor_set_;

	
	void configurePushConstants(VkPipelineLayoutCreateInfo* info,
		VkPushConstantRange* range);
	void configureDescriptorSet(VkPipelineLayoutCreateInfo* info, 
		VkDescriptorSetLayout global_layout,
		VkDescriptorSetLayout global_pbr_layout,
		VkDescriptorSetLayout global_lights_layout);
	void configureAttributes(VkPipelineLayoutCreateInfo* info);
};


#endif // !__LAVA_PIPELINE_H 
