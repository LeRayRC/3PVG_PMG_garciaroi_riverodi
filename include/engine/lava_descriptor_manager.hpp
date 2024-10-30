#ifndef __LAVA_DESCRIPTOR_MANAGER_H__
#define __LAVA_DESCRIPTOR_MANAGER_H__ 1

#include "lava_types.hpp"

class LavaDescriptorManager{
public:
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};
	LavaDescriptorManager();
	LavaDescriptorManager(VkDevice device, uint32_t initial_sets, std::vector<PoolSizeRatio> pool_ratios);
	~LavaDescriptorManager();

	void clearPools();
	void destroyPools();

	VkDescriptorSet allocate(VkDescriptorSetLayout layout, void* pNext = nullptr);
	void writeImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
	void writeBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

	void clear();
	void updateSet(VkDescriptorSet set);
	VkDescriptorPool getPool();
	VkDescriptorPool createPool(uint32_t set_count);
private:

	std::vector<PoolSizeRatio> ratios_;
	std::vector<VkDescriptorPool> fullPools_;
	std::vector<VkDescriptorPool> readyPools_;
	uint32_t setsPerPool_;
	VkDevice device_;

	std::deque<VkDescriptorImageInfo> imageInfos_;
	std::deque<VkDescriptorBufferInfo> bufferInfos_;
	std::vector<VkWriteDescriptorSet> writes_;

	
};

#endif // !__LAVA_DESCRIPTOR_MANAGER_H__
