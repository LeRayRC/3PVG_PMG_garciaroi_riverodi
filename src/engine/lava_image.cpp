#include "lava/engine/lava_image.hpp"
#include "engine/lava_vulkan_inits.hpp"
#include "engine/lava_vulkan_helpers.hpp"
#include "lava/engine/lava_buffer.hpp"
#include "engine/lava_device.hpp"
#include "lava/engine/lava_engine.hpp"
#include "engine/lava_allocator.hpp"

#include "stb_image.h"

LavaImage::LavaImage(LavaEngine* engine,void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped){
	engine_ = engine;
	size_t data_size = size.depth * size.width * size.height * 4;
	LavaBuffer upload_buffer = LavaBuffer(*engine->allocator_, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(upload_buffer.get_buffer().info.pMappedData, data, data_size);

	allocate(size, format, usage, mipmapped);

	engine_->immediate_submit([&](VkCommandBuffer cmd) {
		TransitionImage(cmd, image_.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy_region = {};
		copy_region.bufferOffset = 0;
		copy_region.bufferRowLength = 0;
		copy_region.bufferImageHeight = 0;

		copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.imageSubresource.mipLevel = 0;
		copy_region.imageSubresource.baseArrayLayer = 0;
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageExtent = size;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, upload_buffer.get_buffer().buffer, image_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copy_region);

		TransitionImage(cmd, image_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;

	vkCreateSampler(engine->device_->get_device(), &sampler_info, nullptr, &sampler_);
}


LavaImage::LavaImage(LavaEngine* engine,
	std::string path,
	VkFormat format,
	VkImageUsageFlags usage,
	bool mipmapped) {

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
	assert(data != nullptr);

	engine_ = engine;
	size_t data_size = width * height * 4;

	LavaBuffer upload_buffer = LavaBuffer(*engine->allocator_, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(upload_buffer.get_buffer().info.pMappedData, data, data_size);

	VkExtent3D imagesize;
	imagesize.width = width;
	imagesize.height = height;
	imagesize.depth = 1;
	allocate(imagesize, format, usage, mipmapped);

	engine_->immediate_submit([&](VkCommandBuffer cmd) {
		TransitionImage(cmd, image_.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy_region = {};
		copy_region.bufferOffset = 0;
		copy_region.bufferRowLength = 0;
		copy_region.bufferImageHeight = 0;

		copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.imageSubresource.mipLevel = 0;
		copy_region.imageSubresource.baseArrayLayer = 0;
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageExtent = imagesize;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, upload_buffer.get_buffer().buffer, image_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copy_region);

		TransitionImage(cmd, image_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;

	vkCreateSampler(engine->device_->get_device(), &sampler_info, nullptr, &sampler_);
		
	stbi_image_free(data);
}

LavaImage::LavaImage(LavaEngine* engine,
	VkExtent3D imagesize,
	VkFormat format,
	VkImageUsageFlags usage,
	bool mipmapped,
	int layers,
	VkSamplerCreateInfo* sampler_info_ptr) {

	engine_ = engine;
	allocate(imagesize, format, usage, mipmapped,layers);

	VkSamplerCreateInfo sampler_info = {};
	if (sampler_info_ptr) {
		sampler_info = *sampler_info_ptr;
	}
	else {
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
	}

	vkCreateSampler(engine->device_->get_device(), &sampler_info, nullptr, &sampler_);
}


LavaImage::~LavaImage(){
	vkDestroySampler(engine_->device_->get_device(), sampler_, nullptr);
	vkDestroyImageView(engine_->device_->get_device(), image_.image_view, nullptr);
	vmaDestroyImage(engine_->allocator_->get_allocator(), image_.image, image_.allocation);

}



void LavaImage::allocate(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped, int layers)
{
	image_.image_format = format;
	image_.image_extent = size;

	VkImageCreateInfo img_info = vkinit::ImageCreateInfo(format, usage, size, layers);
	if (mipmapped) {
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY ;
	alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	vmaCreateImage(engine_->allocator_->get_allocator(), &img_info, &alloc_info, &image_.image, &image_.allocation, nullptr);

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build a image-view for the image
	VkImageViewCreateInfo view_info = vkinit::ImageViewCreateInfo(format, image_.image, aspectFlag);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	if (vkCreateImageView(engine_->device_->get_device(), &view_info, nullptr, &image_.image_view) != VK_SUCCESS) {
		printf("Failed to allocate LavaImage\n");
		exit(-1);
	}
	

}