#ifndef __LAVA_SURFACE_H__
#define __LAVA_SURFACE_H__ 1
#endif // !__LAVA_SURFACE_H__

#include "lava/common/lava_types.hpp"

class LavaSurface
{
public:
	LavaSurface(VkInstance instance, GLFWwindow* window);
	~LavaSurface();

	VkSurfaceKHR get_surface() const;

private:
	VkSurfaceKHR surface_;
	VkInstance instance_;
};

