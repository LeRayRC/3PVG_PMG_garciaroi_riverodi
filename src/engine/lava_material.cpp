#include "engine/lava_material.hpp"

LavaMaterial::LavaMaterial(LavaEngine& engine, MaterialProperties prop) : 
  pipeline_{ PipelineConfig(prop.vertex_shader_path,
              prop.fragment_shader_path,
              &engine.device_,
              &engine.swap_chain_,
              engine.draw_image_descriptor_set_layout_,
              prop.pipeline_flags) } {

  name_ = prop.name;
  

}


LavaMaterial::~LavaMaterial(){}

