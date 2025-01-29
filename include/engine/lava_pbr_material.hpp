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
	
	/*VkDescriptorSet get_descriptor_set() {
		return descriptor_set_;
	}*/


	void UpdateGlobalDescriptorSet(LavaBuffer& buffer_properties,
	LavaBuffer& light_buffer_properties, LightShaderStruct& light_parameter) {
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

		buffer_properties.updateBufferData(&uniform_properties, sizeof(LavaPBRMaterialProperties));
		engine_->global_descriptor_allocator_.writeBuffer(4, buffer_properties.get_buffer().buffer, sizeof(LavaPBRMaterialProperties), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		light_buffer_properties.updateBufferData(&light_parameter, sizeof(LightShaderStruct));
		engine_->global_descriptor_allocator_.writeBuffer(5, light_buffer_properties.get_buffer().buffer, sizeof(LightShaderStruct), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	}

private:
	std::string name_;

	std::shared_ptr<LavaImage> base_color_; // if null pick the global base color texture

	std::shared_ptr<LavaImage> metallic_roughness_; // If null pick the global metallic texture (black or white)
	
	std::shared_ptr<LavaImage> opacity_; // If null pick the global opacity mask completely white
	
	std::shared_ptr<LavaImage> normal_; // If null pick the global normal texture

	LavaPBRMaterialProperties uniform_properties;
	

	LavaEngine *engine_;

	friend class LavaMesh;
};




#endif // !__LAVA_MATERIAL_H__ 
