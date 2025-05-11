/**
 * @file lava_inmediate_communication.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Inmediate Communication's file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "engine/lava_inmediate_communication.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_surface.hpp"
#include "lava_vulkan_helpers.hpp"

LavaInmediateCommunication::LavaInmediateCommunication(LavaDevice& use_device, LavaSurface& use_surface)
{
	device_ = &use_device;

	QueueFamilyIndices queueFamilyIndices =
		FindQueueFamilies(device_->get_physical_device(), use_surface.get_surface());

	VkCommandPoolCreateInfo command_pool_info{};
	command_pool_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device_->get_device(), &command_pool_info, nullptr,
		&immediate_command_pool) != VK_SUCCESS) {
		exit(-1);
	}

	VkCommandBufferAllocateInfo immediate_command_buffer_alloc_info{};
	immediate_command_buffer_alloc_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	immediate_command_buffer_alloc_info.pNext = nullptr;
	immediate_command_buffer_alloc_info.commandPool = immediate_command_pool;
	immediate_command_buffer_alloc_info.commandBufferCount = 1;
	immediate_command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//Se reserva los command buffer correspondientes
	if (vkAllocateCommandBuffers(device_->get_device(), &immediate_command_buffer_alloc_info,
		&immediate_command_buffer) != VK_SUCCESS) {
		exit(-1);
	}

	VkFenceCreateInfo fence_info{};
	fence_info.sType =
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;

	//Sync objects for immediate submits

	if (vkCreateFence(device_->get_device(), &fence_info, nullptr, &immediate_fence) != VK_SUCCESS) {
		exit(-1);
	}
}

LavaInmediateCommunication::LavaInmediateCommunication(LavaDevice& use_device)
{
	device_ = &use_device;

	VkCommandPoolCreateInfo command_pool_info{};
	command_pool_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex = use_device.get_queue_family_index();

	if (vkCreateCommandPool(device_->get_device(), &command_pool_info, nullptr,
		&immediate_command_pool) != VK_SUCCESS) {
		exit(-1);
	}

	VkCommandBufferAllocateInfo immediate_command_buffer_alloc_info{};
	immediate_command_buffer_alloc_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	immediate_command_buffer_alloc_info.pNext = nullptr;
	immediate_command_buffer_alloc_info.commandPool = immediate_command_pool;
	immediate_command_buffer_alloc_info.commandBufferCount = 1;
	immediate_command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//Se reserva los command buffer correspondientes
	if (vkAllocateCommandBuffers(device_->get_device(), &immediate_command_buffer_alloc_info,
		&immediate_command_buffer) != VK_SUCCESS) {
		exit(-1);
	}

	VkFenceCreateInfo fence_info{};
	fence_info.sType =
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;

	//Sync objects for immediate submits

	if (vkCreateFence(device_->get_device(), &fence_info, nullptr, &immediate_fence) != VK_SUCCESS) {
		exit(-1);
	}
}

LavaInmediateCommunication::~LavaInmediateCommunication()
{
	vkDestroyCommandPool(device_->get_device(), immediate_command_pool, nullptr);
	vkDestroyFence(device_->get_device(), immediate_fence, nullptr);
}
