#include "engine/lava_descriptor_manager.hpp"


LavaDescriptorManager::LavaDescriptorManager(VkDevice device, 
  uint32_t initial_sets, 
  std::vector<struct PoolSizeRatio> pool_ratios){
  device_ = device;
}

LavaDescriptorManager::LavaDescriptorManager(){}

LavaDescriptorManager::~LavaDescriptorManager()
{
  destroyPools();
}

void LavaDescriptorManager::destroyPools() {
  for (auto pool : readyPools_) {
    vkDestroyDescriptorPool(device_, pool, nullptr);
  }
  readyPools_.clear();
  for (auto pool : fullPools_) {
    vkDestroyDescriptorPool(device_, pool, nullptr);
  } 
  fullPools_.clear();
}

void LavaDescriptorManager::clearPools() {
  for (auto pool : readyPools_) {
    vkResetDescriptorPool(device_, pool, 0);
  }
  for (auto pool : fullPools_) {
    vkResetDescriptorPool(device_, pool, 0);
    readyPools_.push_back(pool);
  }
  fullPools_.clear();
}

VkDescriptorPool LavaDescriptorManager::getPool(){
  VkDescriptorPool new_pool;
  if (readyPools_.size() != 0) {
    new_pool = readyPools_.back();
    readyPools_.pop_back();
  }
  else {
    new_pool = createPool(setsPerPool_);
    setsPerPool_ = setsPerPool_ * 1.5f;
    if (setsPerPool_ > 4092) {
      setsPerPool_ = 4092;
    }
  }

  return new_pool;
}

VkDescriptorPool LavaDescriptorManager::createPool(uint32_t set_count) {
  std::vector<VkDescriptorPoolSize> pool_sizes;
  for (PoolSizeRatio ratio : ratios_) {
    pool_sizes.push_back(VkDescriptorPoolSize{
      .type = ratio.type,
      .descriptorCount = uint32_t(ratio.ratio * set_count)
      });
  }

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = 0;
  pool_info.maxSets = set_count;
  pool_info.poolSizeCount = (uint32_t)pool_sizes.size();
  pool_info.pPoolSizes = pool_sizes.data();

  VkDescriptorPool new_pool;
  vkCreateDescriptorPool(device_, &pool_info, nullptr, &new_pool);
  return new_pool;
}

VkDescriptorSet LavaDescriptorManager::allocate(VkDescriptorSetLayout layout, void* pNext) {
  //get or create a pool to allocate from
  VkDescriptorPool pool_to_use = getPool();

  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.pNext = pNext;
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = pool_to_use;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &layout;

  VkDescriptorSet ds;
  VkResult result = vkAllocateDescriptorSets(device_, &alloc_info, &ds);

  //allocation failed. Try again
  if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

    fullPools_.push_back(pool_to_use);

    pool_to_use = getPool();
    alloc_info.descriptorPool = pool_to_use;

    vkAllocateDescriptorSets(device_, &alloc_info, &ds);
  }

  readyPools_.push_back(pool_to_use);
  return ds;
}


void LavaDescriptorManager::writeBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
  VkDescriptorBufferInfo& info = bufferInfos_.emplace_back(VkDescriptorBufferInfo{
    .buffer = buffer,
    .offset = offset,
    .range = size
    });

  VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

  write.dstBinding = binding;
  write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pBufferInfo = &info;

  writes_.push_back(write);
}

void LavaDescriptorManager::writeImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
  VkDescriptorImageInfo& info = imageInfos_.emplace_back(VkDescriptorImageInfo{
  .sampler = sampler,
  .imageView = image,
  .imageLayout = layout
    });

  VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

  write.dstBinding = binding;
  write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pImageInfo = &info;

  writes_.push_back(write);
}

void LavaDescriptorManager::clear()
{
  imageInfos_.clear();
  writes_.clear();
  bufferInfos_.clear();
}

void LavaDescriptorManager::updateSet(VkDescriptorSet set)
{
  for (VkWriteDescriptorSet& write : writes_) {
    write.dstSet = set;
  }

  vkUpdateDescriptorSets(device_, (uint32_t)writes_.size(), writes_.data(), 0, nullptr);
}