#ifndef __LAVA_PIPELINE_H_
#define __LAVA_PIPELINE_H_ 1

#include "engine/lava_device.hpp"
#include "engine/lava_swap_chain.hpp"
#include "lava/common/lava_types.hpp"

class LavaPipeline
{
public:

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
