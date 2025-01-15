#include "engine/lava_pbr_material.hpp"
#include "engine/lava_image.hpp"

LavaPBRMaterial::LavaPBRMaterial(LavaEngine& engine, MaterialPBRProperties prop){

  name_ = prop.name;
  //descriptor_set_ = descriptor_manager_.allocate(pipeline_.get_descriptor_set_layouts()[1]);
  

  //set_image(0, { true,engine.default_texture_image_.get(), engine.default_texture_image_.get()});
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

