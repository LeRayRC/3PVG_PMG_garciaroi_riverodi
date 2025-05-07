
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
    if (engine.is_session_running()) {
      engine.beginFrame();
      if (engine.is_session_active()) {
        //It is necessary to render in two views
        for (uint32_t i = 0; i < engine.get_view_count(); i++) {
          engine.prepareView(i);

          //RENDER!!!!
          engine.clearColor(i, 1.0f, 0.0f, 0.0f, 1.0f);

          engine.releaseView(i);
        }
      }
      engine.endFrame();
    }
  }


  return 0;
}
