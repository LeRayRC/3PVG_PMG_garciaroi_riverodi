#include "lava/engine/lava_pbr_material.hpp"
#include "lava/engine/lava_image.hpp"
#include "lava/engine/lava_engine.hpp"
#include "lava/vr/lava_engine_vr.hpp"
#include "engine/lava_descriptor_manager.hpp"
#include "lava/engine/lava_buffer.hpp"
#include "engine/lava_device.hpp"

LavaPBRMaterial::LavaPBRMaterial(LavaEngine& engine, MaterialPBRProperties prop) : 
	global_descriptor_allocator_{ *engine .global_descriptor_allocator_.get()}
{
  engine_ = &engine;
	engine_vr_ = nullptr;
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

	position_ = engine.default_texture_image_white;

  pbr_data_buffer_ = std::make_unique<LavaBuffer>(*engine.allocator_, 
    sizeof(LavaPBRMaterialProperties), 
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		engine_->device_.get());

  pbr_data_buffer_->setMappedData();
  descriptor_set_ = engine_->global_descriptor_allocator_->allocate(engine_->global_pbr_descriptor_set_layout_);
  UpdateDescriptorSet();
}

LavaPBRMaterial::LavaPBRMaterial(LavaEngineVR& engine, MaterialPBRProperties prop) :
	global_descriptor_allocator_{ *engine.global_descriptor_allocator_.get() }
{
	engine_ = nullptr;
	engine_vr_ = &engine;
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

	position_ = engine.default_texture_image_white;

	pbr_data_buffer_ = std::make_unique<LavaBuffer>(*engine.allocator_,
		sizeof(LavaPBRMaterialProperties),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		engine.device_.get());

	pbr_data_buffer_->setMappedData();
	descriptor_set_ = engine.global_descriptor_allocator_->allocate(engine.global_pbr_descriptor_set_layout_);
	UpdateDescriptorSet();
}

LavaPBRMaterial::~LavaPBRMaterial(){}


void LavaPBRMaterial::UpdateDescriptorSet() {
	global_descriptor_allocator_.clear();
	global_descriptor_allocator_.writeImage(
		0,
		base_color_->get_allocated_image().image_view,
		base_color_->get_sampler(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	global_descriptor_allocator_.writeImage(
		1,
		normal_->get_allocated_image().image_view,
		normal_->get_sampler(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	global_descriptor_allocator_.writeImage(
		2,
		metallic_roughness_->get_allocated_image().image_view,
		metallic_roughness_->get_sampler(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	global_descriptor_allocator_.writeImage(
		3,
		opacity_->get_allocated_image().image_view,
		opacity_->get_sampler(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	global_descriptor_allocator_.writeImage(
		5,
		position_->get_allocated_image().image_view,
		position_->get_sampler(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);


	pbr_data_buffer_->updateBufferData(&uniform_properties, sizeof(LavaPBRMaterialProperties));
	global_descriptor_allocator_.writeBuffer(4, pbr_data_buffer_->get_buffer().buffer, sizeof(LavaPBRMaterialProperties), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	global_descriptor_allocator_.updateSet(descriptor_set_);
	global_descriptor_allocator_.clear();
}
