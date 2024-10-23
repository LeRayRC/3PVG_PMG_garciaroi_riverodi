#ifndef __LAVA_PIPELINE_H_
#define __LAVA_PIPELINE_H_ 1

#include "engine/lava_device.hpp"
#include "engine/lava_swap_chain.hpp"
#include "lava_types.hpp"

class LavaPipeline
{
public:

	LavaPipeline(PipelineConfig config);
	~LavaPipeline();

	VkPipelineLayout get_layout() { return layout_;}
	VkPipeline get_pipeline() { return pipeline_; }



private:
	int flags_;
	VkPipelineLayout layout_;
	VkPipeline pipeline_;
	VkDevice device_;

	void configurePushConstants(VkPipelineLayoutCreateInfo* info,
		VkPushConstantRange* range);
	void configureDescriptorSet(VkPipelineLayoutCreateInfo* info, 
		VkDescriptorSetLayout layout);
	void configureAttributes(VkPipelineLayoutCreateInfo* info);
};


#endif // !__LAVA_PIPELINE_H 
