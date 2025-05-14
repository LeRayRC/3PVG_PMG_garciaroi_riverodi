#ifndef __LAVA_IMAGE_H__ 
#define __LAVA_IMAGE_H__ 1

#include "lava/common/lava_types.hpp"


class LavaImage
{
public:
	LavaImage(class LavaEngine* engine,
		void* data,
		VkExtent3D size,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		bool mipmapped = false);

	LavaImage(class LavaEngine* engine,
		std::string path,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		bool mipmapped = false);

	LavaImage(class LavaEngine* engine,
		VkExtent3D imagesize,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		bool mipmapped = false,
		int layers = 1,
		VkSamplerCreateInfo* sampler_info_ptr = VK_NULL_HANDLE);

	LavaImage(class LavaEngineVR* engine,
		void* data,
		VkExtent3D size,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		bool mipmapped = false);

	LavaImage(class LavaEngineVR* engine,
		std::string path,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		bool mipmapped = false);

	LavaImage(class LavaEngineVR* engine,
		VkExtent3D imagesize,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		bool mipmapped = false,
		int layers = 1,
		VkSamplerCreateInfo* sampler_info_ptr = VK_NULL_HANDLE);



	~LavaImage();

	AllocatedImage get_allocated_image() {
		return image_;
	}

	VkSampler get_sampler() {
		return sampler_;
	}

private:
	void allocate(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped=false, int layers = 1);
	AllocatedImage image_;
	VkSampler sampler_;
	class LavaEngine* engine_;
	class LavaEngineVR* engine_vr_;
};



#endif // !__LAVA_IMAGE_H__ 
