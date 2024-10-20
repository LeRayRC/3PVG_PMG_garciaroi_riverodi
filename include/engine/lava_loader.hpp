#ifndef  __LAVA_LOADER_H_
#define  __LAVA_LOADER_H_ 1

#include "lava_types.hpp"
#include <unordered_map>
#include <filesystem>





//forward declaration
class LavaEngine;

std::optional<std::vector<std::shared_ptr<MeshAsset>>> LoadGLTFMesh(LavaEngine* engine, 
  std::filesystem::path filePath);


#endif // ! __LAVA_LOADER_H_

