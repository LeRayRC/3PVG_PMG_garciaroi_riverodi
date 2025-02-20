/**
 * @file lava_allocator.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Allocator's header file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_ALLOCATOR_H__
#define __LAVA_ALLOCATOR_H__ 1

#include "lava/common/lava_types.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_instance.hpp"

class LavaAllocator {
public:
	LavaAllocator(LavaDevice& use_device, LavaInstance& use_instance);
	~LavaAllocator();

	VmaAllocator get_allocator() { return allocator_; }

private:
	VmaAllocator allocator_;
};

#endif