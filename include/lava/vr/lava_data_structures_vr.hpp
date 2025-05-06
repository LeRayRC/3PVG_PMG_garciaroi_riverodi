#ifndef __LAVA_DATA_STRUCTURES_VR_H__
#define __LAVA_DATA_STRUCTURES_VR_H__ 1

#include <openxr/openxr.h>

struct RenderLayerInfo {
  XrTime predicted_display_time;
  std::vector<XrCompositionLayerBaseHeader*> layers;
  XrCompositionLayerProjection layer_projection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
  std::vector<XrCompositionLayerProjectionView> layer_projection_views;
};


#endif // !__LAVA_DATA_STRUCTURES_VR_H__
