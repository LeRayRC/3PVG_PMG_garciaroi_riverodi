#include "lava_descriptors.hpp"

void DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type) {
  VkDescriptorSetLayoutBinding bind{};
  bind.binding = binding;
  bind.descriptorCount = 1;
  bind.descriptorType = type;
  bindings_.push_back(bind);
}
void DescriptorLayoutBuilder::clear() {
  bindings_.clear();
}
VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, 
  VkShaderStageFlags shader_stages,
  void* pnext, 
  VkDescriptorSetLayoutCreateFlags flags) {
  for (auto& b : bindings_) {
    b.stageFlags |= shader_stages;
  }
  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.pNext = pnext;
  info.pBindings = bindings_.data();
  info.bindingCount = (uint32_t)bindings_.size();
  info.flags = flags;

  VkDescriptorSetLayout set;
  if (vkCreateDescriptorSetLayout(device, &info, nullptr, &set) != VK_SUCCESS) {
#ifndef NDEBUG
    printf("Descriptor set layout creation failed!");
#endif // !NDEBUG
  };

  return set;
}

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
  //Lo he tenido que añadir para poder borrarlo sin problemas
  pool_info.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

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
