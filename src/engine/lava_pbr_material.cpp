#include "engine/lava_pbr_material.hpp"
#include "engine/lava_image.hpp"

LavaPBRMaterial::LavaPBRMaterial(LavaEngine& engine, MaterialPBRProperties prop){

  engine_ = &engine;
  name_ = prop.name;
  base_color_ = engine.default_texture_image_pink;
  
  metallic_roughness_ = engine.default_texture_image_white;
  uniform_properties.metallic_factor_ = 0.5f;
  uniform_properties.roughness_factor_ = 0.5f;

  uniform_properties.specular_factor_ = 0.5f; //Maybe Wrong

  opacity_ = engine.default_texture_image_white;
  uniform_properties.opacity_mask_ = 0.5f;

  normal_ = engine.default_texture_image_white;
  uniform_properties.use_normal_ = 0.0f;

  pbr_data_buffer_ = std::make_unique<LavaBuffer>(engine.allocator_, 
    sizeof(LavaPBRMaterialProperties), 
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    &engine_->device_);
  pbr_data_buffer_->setMappedData();
  descriptor_set_ = engine_->global_descriptor_allocator_.allocate(engine_->global_pbr_descriptor_set_layout_);
  UpdateDescriptorSet();
}

//LavaMaterialImage LavaMaterial::get_image(unsigned int index) {
//  if (index > (unsigned int)images_.size()) index = (unsigned int)images_.size() - 1;
//  return images_[index];
//}

//Para esto se va a hacer un funcion con un enum donde se pueda hacer set de una imagen
//u otra con un switch para poder actualizar la imagen de textura normal, oclussion, etc..
//void LavaPBRMaterial::set_image(unsigned int index, LavaMaterialImage temp_image) {
//  if (index > images_.size()) index = (unsigned int)(images_.size() - 1);
//  LavaMaterialImage& image = images_[index];
//  image.active = temp_image.active;
//  image.diffuse = temp_image.diffuse;
//  image.normal = temp_image.normal;
//
//  //Once an image is set, update the descriptor set
//  descriptor_manager_.clear();
//  if (image.diffuse) {
//
//  descriptor_manager_.writeImage(
//    0,
//    image.diffuse->get_allocated_image().image_view,
//    image.diffuse->get_sampler(),
//    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
//  }
//
//  if (image.normal) {
//
//    descriptor_manager_.writeImage(
//      1,
//      image.normal->get_allocated_image().image_view,
//      image.normal->get_sampler(),
//      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
//  }
//  descriptor_manager_.updateSet(descriptor_set_);
//  descriptor_manager_.clear();
//}


LavaPBRMaterial::~LavaPBRMaterial(){}

