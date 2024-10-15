#ifndef __LAVA_PIPELINE_H_
#define __LAVA_PIPELINE_H_ 1

#include "engine/lava_device.hpp"
#include "engine/lava_swap_chain.hpp"

class LavaPipeline
{
public:

	struct Config {
	public:
		const char* vertex_shader_path;
		const char* fragment_shader_path;
		LavaDevice* device;
		LavaSwapChain* swap_chain;
		VkDescriptorSetLayout descriptor_set_layout;
		int pipeline_flags;
	};


	LavaPipeline(Config& config);
	~LavaPipeline();

	

	typedef enum PipelineFlags {
		PIPELINE_USE_ATTRIBUTES = 1,					//0001
		PIPELINE_USE_PUSHCONSTANTS = 1 << 1,	//0010
		PIPELINE_USE_DESCRIPTOR_SET = 1 << 2,  //0100
	};

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
