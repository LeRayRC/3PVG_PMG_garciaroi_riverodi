
#include "lava/vr/lava_engine_vr.hpp"



int main(int argc, char** argv) {
#ifdef XR_USE_GRAPHICS_API_VULKAN
  printf("Vulkan use!!\n");
#endif
  LavaEngineVR engine;

  //while (!engine.shouldClose()) {
  //  engine.pollEvents();
  //}


  return 0;
}
