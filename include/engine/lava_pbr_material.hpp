#ifndef __LAVA_PBR_MATERIAL_H__ 
#define __LAVA_PBR_MATERIAL_H__ 1

#include "lava_types.hpp"
#include "ecs/lava_ecs_components.hpp"
#include "lava_engine.hpp"


struct LavaPBRMaterialProperties {
	float metallic_factor_; //Determines the metallic value of the metallic parts of the texture
	float roughness_factor_; //Determines the roughness value of the metallic parts of the texture Default value 0.5f
	float specular_factor_;
	float opacity_mask_;
	float use_normal_; //Determines to use 
};


class LavaPBRMaterial
{
public:
	LavaPBRMaterial(class LavaEngine& engine, MaterialPBRProperties prop);
	~LavaPBRMaterial();

	std::string get_name() { return name_; }
	
	VkDescriptorSet get_descriptor_set() {
		return descriptor_set_;
	}

	void UpdateBaseColorImage(std::shared_ptr<LavaImage> image) {
		base_color_ = image;
		UpdateDescriptorSet();
	}

	void UpdateBaseMetallicRoughnessImage(std::shared_ptr<LavaImage> image) {
		metallic_roughness_ = image;
		UpdateDescriptorSet();
	}

	void UpdateOpacityImage(std::shared_ptr<LavaImage> image) {
		opacity_ = image;
		UpdateDescriptorSet();
	}

	void UpdateNormalImage(std::shared_ptr<LavaImage> image) {
		normal_ = image;
		UpdateDescriptorSet();
	}

	void UpdateDescriptorSet() {
		engine_->global_descriptor_allocator_.clear();
		engine_->global_descriptor_allocator_.writeImage(
			0,
			base_color_->get_allocated_image().image_view,
			base_color_->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		engine_->global_descriptor_allocator_.writeImage(
			1,
			normal_->get_allocated_image().image_view,
			normal_->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		engine_->global_descriptor_allocator_.writeImage(
			2,
			metallic_roughness_->get_allocated_image().image_view,
			metallic_roughness_->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		engine_->global_descriptor_allocator_.writeImage(
			3,
			opacity_->get_allocated_image().image_view,
			opacity_->get_sampler(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		pbr_data_buffer_->updateBufferData(&uniform_properties, sizeof(LavaPBRMaterialProperties));
		engine_->global_descriptor_allocator_.writeBuffer(4, pbr_data_buffer_->get_buffer().buffer, sizeof(LavaPBRMaterialProperties), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		engine_->global_descriptor_allocator_.updateSet(descriptor_set_);
		engine_->global_descriptor_allocator_.clear();
	}

private:
	std::string name_;

	std::shared_ptr<LavaImage> base_color_; // if null pick the global base color texture

	std::shared_ptr<LavaImage> metallic_roughness_; // If null pick the global metallic texture (black or white)
	
	std::shared_ptr<LavaImage> opacity_; // If null pick the global opacity mask completely white
	
	std::shared_ptr<LavaImage> normal_; // If null pick the global normal texture

	LavaPBRMaterialProperties uniform_properties;
	
	VkDescriptorSet descriptor_set_;
	std::unique_ptr<LavaBuffer> pbr_data_buffer_;


	LavaEngine *engine_;

	friend class LavaMesh;
};




#endif // !__LAVA_MATERIAL_H__ 
