#include "lava/common/lava_global_helpers.hpp"


const char* GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type) {
#if defined(XR_USE_GRAPHICS_API_D3D11)
  if (type == D3D11) {
    return XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
  }
#endif
#if defined(XR_USE_GRAPHICS_API_D3D12)
  if (type == D3D12) {
    return XR_KHR_D3D12_ENABLE_EXTENSION_NAME;
  }
#endif
#if defined(XR_USE_GRAPHICS_API_OPENGL)
  if (type == OPENGL) {
    return XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
  }
#endif
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
  if (type == OPENGL_ES) {
    return XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME;
  }
#endif
#if defined(XR_USE_GRAPHICS_API_VULKAN)
  if (type == VULKAN) {
    return "XR_KHR_vulkan_enable";
  }
#endif
  std::cerr << "ERROR: Unknown Graphics API." << std::endl;
  DEBUG_BREAK;
  return nullptr;
}