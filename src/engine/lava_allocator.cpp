/**
 * @file lava_allocator.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Allocator's file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "engine/lava_allocator.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_instance.hpp"

LavaAllocator::LavaAllocator(LavaDevice& use_device, LavaInstance& use_instance)
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = use_device.get_physical_device();
	allocatorInfo.device = use_device.get_device();
	allocatorInfo.instance = use_instance.get_instance();
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &allocator_);
}

LavaAllocator::~LavaAllocator()
{
	vmaDestroyAllocator(allocator_);
}
