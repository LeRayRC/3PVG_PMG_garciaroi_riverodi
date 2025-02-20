#ifndef __LAVA_IMAGE_H__ 
#define __LAVA_IMAGE_H__ 1

#include "lava/common/lava_types.hpp"
//#include "lava_engine.hpp"

class LavaImage
{
public:
	LavaImage(class LavaEngine* engine,void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped=false);
	~LavaImage();

	AllocatedImage get_allocated_image() {
		return image_;
	}

	VkSampler get_sampler() {
		return sampler_;
	}

private:
	void allocate(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped=false);
	AllocatedImage image_;
	VkSampler sampler_;
	class LavaEngine* engine_;
};



#endif // !__LAVA_IMAGE_H__ 
