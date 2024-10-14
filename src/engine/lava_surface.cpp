#include "engine/lava_surface.hpp"

LavaSurface::LavaSurface(VkInstance instance, GLFWwindow* window) {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface_)
    != VK_SUCCESS) {
#ifndef NDEBUG
    printf("failed to create window surface!!\n");
#endif // !NDEBUG
  }
  instance_ = instance;
}

LavaSurface::~LavaSurface(){
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

VkSurfaceKHR LavaSurface::get_surface() const {
  return surface_;
}
