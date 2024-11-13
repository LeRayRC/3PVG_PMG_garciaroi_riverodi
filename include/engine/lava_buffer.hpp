#ifndef __LAVA_BUFFER_H__
#define __LAVA_BUFFER_H__ 1

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"


struct AllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

class LavaBuffer
{
public:
	LavaBuffer();
	LavaBuffer(class LavaAllocator& allocator, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
	~LavaBuffer();

	AllocatedBuffer get_buffer() { return buffer_; }
	void setMappedData(void* data);
	AllocatedBuffer buffer_;
	
private:
	class LavaAllocator* allocator_;
	bool initialized_;
	bool mapped_;
};




#endif // !__LAVA_BUFFER_H__
