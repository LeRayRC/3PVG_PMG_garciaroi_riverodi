/**
 * @file lava_swap_chain.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Swap Chain's file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "engine/lava_swap_chain.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_surface.hpp"
#include "lava_vulkan_inits.hpp"

//LavaSwapChain::LavaSwapChain()
//{
//}

LavaSwapChain::LavaSwapChain(LavaDevice& use_device, LavaSurface& use_surface, VkExtent2D window_extent, VmaAllocator allocator)
{
	device_ = &use_device;
	allocator_ = allocator;
	createSwapChain(use_surface, window_extent);
	createImageViews();
}

void LavaSwapChain::createSwapChain(class LavaSurface& use_surface, VkExtent2D window_extent)
{
	SwapChainSupportDetails swapChainSupport =
		QuerySwapChainSupport(device_->get_physical_device(), use_surface.get_surface());
	//CAREFUL 
	//En una guia recomiendan VK_FORMAT_B8G8R8A8_UNORM (https://vkguide.dev) 
	//Y en otra VK_FORMAT_B8G8R8A8_SRGV
	VkSurfaceFormatKHR surfaceFormat =
		ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		ChooseSwapPresentMode(swapChainSupport.presentModes);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount >
		swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = use_surface.get_surface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = window_extent;
	createInfo.imageArrayLayers = 1;
	//VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	//VK_IMAGE_USAGE_TRANSFER_DST_BIT util para postprocesos
	createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(device_->get_physical_device(), use_surface.get_surface());
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device_->get_device(), &createInfo,
		nullptr, &swap_chain_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!!");
	}

	vkGetSwapchainImagesKHR(device_->get_device(), swap_chain_, &imageCount, nullptr);
	swap_chain_images_.resize(imageCount);
	vkGetSwapchainImagesKHR(device_->get_device(), swap_chain_, &imageCount, swap_chain_images_.data());
	swap_chain_image_format_ = surfaceFormat.format;

	VkExtent3D draw_image_extent = {
		window_extent.width,
		window_extent.height,
		1
	};

	draw_image_.image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
	draw_image_.image_extent = draw_image_extent;

	draw_extent_ = window_extent;

	VkImageUsageFlags draw_image_usages{};
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = vkinit::ImageCreateInfo(draw_image_.image_format,
		draw_image_usages, draw_image_extent);

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(allocator_, &rimg_info, &rimg_allocinfo, &draw_image_.image,
		&draw_image_.allocation, nullptr);

	VkImageViewCreateInfo rview_info = vkinit::ImageViewCreateInfo(draw_image_.image_format,
		draw_image_.image, VK_IMAGE_ASPECT_COLOR_BIT);
	if (vkCreateImageView(device_->get_device(), &rview_info, nullptr, &draw_image_.image_view) !=
		VK_SUCCESS) {
		printf("Error creating image view!\n");
	}

	depth_image_.image_format = VK_FORMAT_D32_SFLOAT;
	depth_image_.image_extent = draw_image_extent;
	VkImageUsageFlags depth_image_usages{};
	depth_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo depth_img_info = vkinit::ImageCreateInfo(depth_image_.image_format,
		depth_image_usages, draw_image_extent);

	//allocate and create the image
	vmaCreateImage(allocator_, &depth_img_info, &rimg_allocinfo, &depth_image_.image, &depth_image_.allocation, nullptr);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo depth_view_info = vkinit::ImageViewCreateInfo(depth_image_.image_format, depth_image_.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	if (vkCreateImageView(device_->get_device(), &depth_view_info, nullptr, &depth_image_.image_view) !=
		VK_SUCCESS) {
		printf("Error creating depth image view!\n");
	}


	//Create shadow map on the swapchain
	shadowmap_image_.image_format = VK_FORMAT_D32_SFLOAT;
	shadowmap_image_.image_extent = draw_image_extent;
	VkImageUsageFlags shadowmap_image_usages{};
	shadowmap_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	shadowmap_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo shadowmap_img_info = vkinit::ImageCreateInfo(shadowmap_image_.image_format,
		shadowmap_image_usages, draw_image_extent);

	//allocate and create the image
	vmaCreateImage(allocator_, &shadowmap_img_info, &rimg_allocinfo, &shadowmap_image_.image, &shadowmap_image_.allocation, nullptr);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo shadowmap_view_info = vkinit::ImageViewCreateInfo(shadowmap_image_.image_format, shadowmap_image_.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	if (vkCreateImageView(device_->get_device(), &shadowmap_view_info, nullptr, &shadowmap_image_.image_view) !=
		VK_SUCCESS) {
		printf("Error creating shadowmap image view!\n");
	}

	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR; 
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.anisotropyEnable = VK_FALSE;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE; // Sin PCF
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f; // Sin mipmaps


	vkCreateSampler(device_->get_device(), &sampler_info, nullptr, &shadowmap_sampler_);


}

void LavaSwapChain::createImageViews()
{
	swap_chain_image_views_.resize(swap_chain_images_.size());
	for (size_t i = 0; i < swap_chain_images_.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swap_chain_images_[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swap_chain_image_format_;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device_->get_device(), &createInfo,
			nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}
}


LavaSwapChain::~LavaSwapChain()
{
	vkDestroySampler(device_->get_device(), shadowmap_sampler_, nullptr);
	vkDestroyImageView(device_->get_device(), shadowmap_image_.image_view, nullptr);
	vmaDestroyImage(allocator_, shadowmap_image_.image, shadowmap_image_.allocation);
	//Destroy Swap Chain
	vkDestroyImageView(device_->get_device(), depth_image_.image_view, nullptr);
	vmaDestroyImage(allocator_, depth_image_.image, depth_image_.allocation);
	vkDestroyImageView(device_->get_device(), draw_image_.image_view, nullptr);
	vmaDestroyImage(allocator_, draw_image_.image, draw_image_.allocation);
	

	for (auto imageView : swap_chain_image_views_) {
		vkDestroyImageView(device_->get_device(), imageView, nullptr);
	}
	vkDestroySwapchainKHR(device_->get_device(), swap_chain_, nullptr);
}

