#include "engine/lava_image.hpp"
#include "lava_vulkan_inits.hpp"
#include "lava_vulkan_helpers.hpp"

LavaImage::LavaImage(LavaEngine* engine,void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped){
	engine_ = engine;
	size_t data_size = size.depth * size.width * size.height * 4;
	AllocatedBuffer upload_buffer = engine_->createBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(upload_buffer.info.pMappedData, data, data_size);

	allocate(size,format, usage, mipmapped);

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
		vkCmdCopyBufferToImage(cmd, upload_buffer.buffer, image_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copy_region);

		TransitionImage(cmd, image_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

	engine->destroyBuffer(upload_buffer);
}

LavaImage::~LavaImage(){
	vkDestroyImageView(engine_->device_.get_device(), image_.image_view, nullptr);
	vmaDestroyImage(engine_->allocator_.get_allocator(), image_.image, image_.allocation);
}



void LavaImage::allocate(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	image_.image_format = format;
	image_.image_extent = size;

	VkImageCreateInfo img_info = vkinit::ImageCreateInfo(format, usage, size);
	if (mipmapped) {
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo alloc_info = {};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY ;
	alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	vmaCreateImage(engine_->allocator_.get_allocator(), &img_info, &alloc_info, &image_.image, &image_.allocation, nullptr);

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build a image-view for the image
	VkImageViewCreateInfo view_info = vkinit::ImageViewCreateInfo(format, image_.image, aspectFlag);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	if (vkCreateImageView(engine_->device_.get_device(), &view_info, nullptr, &image_.image_view) != VK_SUCCESS) {
		printf("Failed to allocate LavaImage\n");
		exit(-1);
	}
	

}