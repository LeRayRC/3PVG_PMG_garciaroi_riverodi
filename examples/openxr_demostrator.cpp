
#include "lava/vr/lava_engine_vr.hpp"
#include <openxr/openxr.h>


int main(int argc, char** argv) {
#ifdef XR_USE_GRAPHICS_API_VULKAN
  printf("Vulkan use!!\n");
#endif
  XrPosef reference_pose = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };

  LavaEngineVR engine(reference_pose);

  while (!engine.shouldClose()) {
    engine.pollEvents();
    engine.beginFrame();
    engine.endFrame();
  }


  return 0;
}
