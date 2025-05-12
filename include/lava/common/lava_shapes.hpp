#ifndef __LAVA_SHAPES_H__ 
#define __LAVA_SHAPES_H__ 1


#include "lava/engine/lava_engine.hpp"
#include "lava/engine/lava_mesh.hpp"

enum class GeometryShape {
  Quad,
  Cube8v,
  Cube24v,
  Sphere,
  Terrain,
  Default
};

std::shared_ptr<LavaMesh> CreateQuad(LavaEngine& engine,
  class LavaPBRMaterial* material, float size = 1.0f);

std::shared_ptr<LavaMesh> CreateCube8v(
  class LavaPBRMaterial* material , float size = 0.1f);

std::shared_ptr<LavaMesh> CreateCube24v(LavaEngine& engine, 
  class LavaPBRMaterial* material, float size = 0.1f);

std::shared_ptr<LavaMesh> CreateSphere(LavaEngine& engine, 
  LavaPBRMaterial* material,float sphere_size = 0.1f, 
  int num_heights = 20, int num_revs = 30);

std::shared_ptr<LavaMesh> CreateTerrain(
  LavaPBRMaterial* material,
  int num_cols = 32, int num_rows = 32,
  float height_mult = 10.0f,
  float size = 1.0f,
  float smoothness = 0.3f,
  glm::vec2 tilling = {1.0, 1.0f},
  bool is_centered = true);




#endif // !__LAVA_SHAPES_H__ 
