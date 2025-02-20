#include "lava/engine/lava_descriptors.hpp"



void DescriptorAllocator::init_pool(VkDevice device, 
  uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios){

  std::vector<VkDescriptorPoolSize> pool_sizes;
  for (PoolSizeRatio ratio : pool_ratios) {
    pool_sizes.push_back(VkDescriptorPoolSize{
        .type = ratio.type,
        .descriptorCount = uint32_t(ratio.ratio * max_sets)
      });
  }

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = 0;
  //Esto significa que el número total de descriptores de un tipo específico
  // (VkDescriptorPoolSize::type) será proporcional al número máximo de sets 
  // (max_sets), según el ratio que has definido para ese tipo en particular.
  //Por eso se multiplica el ratio * max_sets
  pool_info.maxSets = max_sets;
  pool_info.poolSizeCount = (uint32_t)pool_sizes.size();
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

  vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
}
void DescriptorAllocator::clear_descriptors(VkDevice device){
  vkResetDescriptorPool(device, pool, 0);
}
void DescriptorAllocator::destroy_pool(VkDevice device){
  vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout)
{
  VkDescriptorSetAllocateInfo allocInfo;

  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.descriptorPool = pool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &layout;

  VkDescriptorSet ds;
  if (vkAllocateDescriptorSets(device, &allocInfo, &ds) != VK_SUCCESS) {
#ifndef NDEBUG
    printf("Descriptor set allocation failed!");
#endif // !NDEBUG
  }

  return ds;
}
