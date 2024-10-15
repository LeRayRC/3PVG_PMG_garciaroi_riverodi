/**
 * @file lava_frame_data.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Frame Data's file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "engine/lava_frame_data.hpp"
#include "lava_vulkan_helpers.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_surface.hpp"

LavaFrameData::LavaFrameData(LavaDevice& use_device, LavaSurface& use_surface)
{
	device_ = &use_device;
	frame_number_ = 0;

	QueueFamilyIndices queueFamilyIndices =
		FindQueueFamilies(use_device.get_physical_device(), use_surface.get_surface());

	VkCommandPoolCreateInfo command_pool_info{};
	command_pool_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.pNext = nullptr;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value();

	VkFenceCreateInfo fence_info{};
	fence_info.sType =
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		if (vkCreateCommandPool(use_device.get_device(), &command_pool_info, nullptr,
			&frames_[i].command_pool) != VK_SUCCESS) {
			exit(-1);
		}
		VkCommandBufferAllocateInfo command_alloc_info{};
		command_alloc_info.sType =
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_alloc_info.pNext = nullptr;
		command_alloc_info.commandPool = frames_[i].command_pool;
		command_alloc_info.commandBufferCount = 1;
		command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		//Se reserva los command buffer correspondientes
		if (vkAllocateCommandBuffers(use_device.get_device(), &command_alloc_info,
			&frames_[i].main_command_buffer) != VK_SUCCESS) {
			exit(-1);
		}

		if (vkCreateFence(use_device.get_device(), &fence_info, nullptr, &frames_[i].render_fence) != VK_SUCCESS) {
			exit(-1);
		}
		if (vkCreateSemaphore(use_device.get_device(), &semaphore_info, nullptr, &frames_[i].render_semaphore) != VK_SUCCESS) {
			exit(-1);
		}

		if (vkCreateSemaphore(use_device.get_device(), &semaphore_info, nullptr, &frames_[i].swap_chain_semaphore) != VK_SUCCESS) {
			exit(-1);
		}

	}
}

LavaFrameData::~LavaFrameData()
{
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		vkDestroyCommandPool(device_->get_device(), frames_[i].command_pool, nullptr);
		vkDestroyFence(device_->get_device(), frames_[i].render_fence, nullptr);
		vkDestroySemaphore(device_->get_device(), frames_[i].render_semaphore, nullptr);
		vkDestroySemaphore(device_->get_device(), frames_[i].swap_chain_semaphore, nullptr);
	}
}
