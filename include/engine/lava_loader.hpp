#ifndef  __LAVA_LOADER_H_
#define  __LAVA_LOADER_H_ 1

#include "lava_types.hpp"
#include <unordered_map>
#include <filesystem>


struct GeoSurface {
  uint32_t start_index;
  uint32_t count;
};

struct MeshAsset {
  std::string name;

  std::vector<GeoSurface> surfaces;
  GPUMeshBuffers meshBuffers;
};

//forward declaration
class LavaEngine;

std::optional<std::vector<std::shared_ptr<MeshAsset>>> LoadGLTFMesh(LavaEngine* engine, 
  std::filesystem::path filePath);


#endif // ! __LAVA_LOADER_H_

