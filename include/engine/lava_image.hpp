#ifndef __LAVA_IMAGE_H__ 
#define __LAVA_IMAGE_H__ 1

#include "lava_types.hpp"
#include "lava_engine.hpp"

class LavaImage
{
public:
	LavaImage(LavaEngine* engine,void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped=false);
	~LavaImage();

private:
	void allocate(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped=false);
	AllocatedImage image_;
	LavaEngine* engine_;
};



#endif // !__LAVA_IMAGE_H__ 
