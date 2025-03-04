#ifndef __LAVA_DESCRIPTOR_MANAGER_H__
#define __LAVA_DESCRIPTOR_MANAGER_H__ 1

#include "lava/common/lava_types.hpp"

struct DescriptorLayoutBuilder {
	std::vector<VkDescriptorSetLayoutBinding> bindings_;

	void addBinding(uint32_t binding, VkDescriptorType type);
	void clear();
	VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shader_stages,
		void* pnext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

class LavaDescriptorManager{
public:
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};
	LavaDescriptorManager();
	LavaDescriptorManager(VkDevice device,const uint32_t initial_sets, std::vector<PoolSizeRatio> pool_ratios);
	~LavaDescriptorManager();

	void clearPools();
	void destroyPools();

	VkDescriptorSet allocate(VkDescriptorSetLayout layout, void* pNext = nullptr);
	void freeDescriptorSet(VkDescriptorSet descriptor_set_to_free);
	void writeImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
	void writeBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

	void clear();
	void updateSet(VkDescriptorSet set);
	VkDescriptorPool getPool();
	VkDescriptorPool createPool(uint32_t set_count);


	static const std::vector<PoolSizeRatio> pool_ratios;
	static const uint32_t initial_sets = 1000;

private:

	std::vector<PoolSizeRatio> ratios_;
	std::vector<VkDescriptorPool> fullPools_;
	std::vector<VkDescriptorPool> readyPools_;
	uint32_t setsPerPool_;
	VkDevice device_;
	std::map<VkDescriptorSet, VkDescriptorPool> descriptor_set_map_;

	std::deque<VkDescriptorImageInfo> imageInfos_;
	std::deque<VkDescriptorBufferInfo> bufferInfos_;
	std::vector<VkWriteDescriptorSet> writes_;

	
};

#endif // !__LAVA_DESCRIPTOR_MANAGER_H__
