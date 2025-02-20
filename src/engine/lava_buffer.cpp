#include "lava/engine/lava_buffer.hpp"
#include "engine/lava_allocator.hpp"
#include "engine/lava_device.hpp"


#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"



LavaBuffer::LavaBuffer() {
	printf("Default constructor for LavaBuffer\n");
	initialized_ = false;
}



LavaBuffer::LavaBuffer(LavaAllocator& allocator, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, LavaDevice* device)
{
	device_ = device;
	allocator_ = &allocator;
	// allocate buffer
	VkBufferCreateInfo buffer_info = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.pNext = nullptr;
	buffer_info.size = alloc_size;

	buffer_info.usage = usage;

	VmaAllocationCreateInfo vmaalloc_info = {};
	vmaalloc_info.usage = memory_usage ; 
	vmaalloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	// allocate the buffer
	if (vmaCreateBuffer(allocator_->get_allocator(), &buffer_info, &vmaalloc_info, &buffer_.buffer, &buffer_.allocation,
		&buffer_.info)) {
#ifndef NDEBUG
		printf("Mesh Buffer creation fail!");
#endif // !NDEBUG
	}
	initialized_ = true;
	mapped_ = false;

	//setMappedData();
}


LavaBuffer::~LavaBuffer(){
	if (initialized_) {
		if (mapped_) {
			vmaUnmapMemory(allocator_->get_allocator(), buffer_.allocation);
		}
		if (device_) {
			vkDeviceWaitIdle(device_->get_device());
		}
		vmaDestroyBuffer(allocator_->get_allocator(), buffer_.buffer, buffer_.allocation);
	}
	
}

void LavaBuffer::setMappedData() {
	vmaMapMemory(allocator_->get_allocator(), buffer_.allocation, &mapped_data_);
	mapped_ = true;
}

void LavaBuffer::updateBufferData(void* data, size_t size){
	memcpy(mapped_data_, data, size);
}
