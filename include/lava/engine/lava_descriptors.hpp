/*****************************************************************//**
 * \file   lava_descriptors.hpp
 * \brief  
 * 
 * \author Carlos Garcia Roig && Daniel Rivero Diaz
 * \date   October 2024
 *********************************************************************/
#ifndef __LAVA_DESCRIPTORS_H_
#define __LAVA_DESCRIPTORS_H_ 1


#include "lava/common/lava_types.hpp"



struct DescriptorAllocator {
  struct PoolSizeRatio {
    VkDescriptorType type;
    float ratio;
  };
  VkDescriptorPool pool;

  void init_pool(VkDevice device, uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios);
  void clear_descriptors(VkDevice device);
  void destroy_pool(VkDevice device);

  VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

#endif // !__LAVA_DESCRIPTORS_H_
