
#include "lava/openxr_common/DebugOutput.h"
#include "GraphicsAPI_Vulkan.h"
#include "lava/openxr_common/OpenXRDebugUtils.h"

class OpenXRTutorial {

public:
  OpenXRTutorial(GraphicsAPI_Type apiType)
  {
  }
  ~OpenXRTutorial() = default;
  void Run()
  {
  }
private:
  void PollSystemEvents()
  {
  }
private:
  bool m_applicationRunning = true;
  bool m_sessionRunning = false;
};


void OpenXRTutorial_Main(GraphicsAPI_Type apiType) {
  DebugOutput debugOutput;  // This redirects std::cerr and std::cout to the IDE's output or Android Studio's logcat.
  XR_TUT_LOG("OpenXR Tutorial Chapter 2");
  OpenXRTutorial app(apiType);
  app.Run();
}

int main(int argc, char** argv) {
  OpenXRTutorial_Main(VULKAN);
  return 0;
}
