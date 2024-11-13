#include "engine/lava_material.hpp"

LavaMaterial::LavaMaterial(LavaEngine& engine, MaterialProperties prop) : 
  pipeline_{ PipelineConfig(prop.vertex_shader_path,
              prop.fragment_shader_path,
              &engine.device_,
              &engine.swap_chain_,
              engine.global_descriptor_set_layout_,
              prop.pipeline_flags) } {

  name_ = prop.name;
  

}

LavaMaterialImage LavaMaterial::get_image(unsigned int index) {
  if (index > (unsigned int)images_.size()) index = (unsigned int)images_.size() - 1;
  return images_[index];
}

void LavaMaterial::set_image(unsigned int index, LavaMaterialImage temp_image) {
  if (index > images_.size()) index = (unsigned int)(images_.size() - 1);
  LavaMaterialImage& image = images_[index];
  image.active = temp_image.active;
  image.diffuse = temp_image.diffuse;
  image.normal = temp_image.normal;
}


LavaMaterial::~LavaMaterial(){}

