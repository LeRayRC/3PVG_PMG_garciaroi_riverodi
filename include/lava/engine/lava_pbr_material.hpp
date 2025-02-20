#ifndef __LAVA_PBR_MATERIAL_H__ 
#define __LAVA_PBR_MATERIAL_H__ 1

#include "lava/common/lava_types.hpp"
#include "lava/ecs/lava_ecs_components.hpp"
#include "lava/engine/lava_engine.hpp"

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

	void UpdateBaseColorImage(std::shared_ptr<class LavaImage> image) {
		base_color_ = image;
		UpdateDescriptorSet();
	}

	void UpdateBaseMetallicRoughnessImage(std::shared_ptr<class LavaImage> image) {
		metallic_roughness_ = image;
		UpdateDescriptorSet();
	}

	void UpdateOpacityImage(std::shared_ptr<class LavaImage> image) {
		opacity_ = image;
		UpdateDescriptorSet();
	}

	void UpdateNormalImage(std::shared_ptr<class LavaImage> image) {
		normal_ = image;
		UpdateDescriptorSet();
	}

	void UpdateDescriptorSet();

private:
	std::string name_;

	std::shared_ptr<class LavaImage> base_color_; // if null pick the global base color texture

	std::shared_ptr<class LavaImage> metallic_roughness_; // If null pick the global metallic texture (black or white)
	
	std::shared_ptr<class LavaImage> opacity_; // If null pick the global opacity mask completely white
	
	std::shared_ptr<class LavaImage> normal_; // If null pick the global normal texture

	LavaPBRMaterialProperties uniform_properties;
	
	VkDescriptorSet descriptor_set_;
	std::unique_ptr<class LavaBuffer> pbr_data_buffer_;


	LavaEngine *engine_;

	friend class LavaMesh;
};




#endif // !__LAVA_MATERIAL_H__ 
