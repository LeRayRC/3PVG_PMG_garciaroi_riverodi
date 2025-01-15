#ifndef __LAVA_PBR_MATERIAL_H__ 
#define __LAVA_PBR_MATERIAL_H__ 1

#include "lava_types.hpp"
#include "lava_engine.hpp"


struct LavaPBRMaterialProperties {

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
//TODO CHANGE TO PRIVATE
	public:
	std::string name_;
	//VkDescriptorSet descriptor_set_;
	//LavaDescriptorManager& descriptor_manager_;
	std::shared_ptr<LavaImage> base_color_; // if null pick the global base color texture
	std::shared_ptr<LavaImage> metallic_; // If null pick the global metallic texture (black or white)
	float metallic_factor_;	//Determines the metallic value of the metallic parts of the texture
	std::shared_ptr<LavaImage> roughness_; // If null pick the global roughness texture
	float roughness_factor_;	//Determines the roughness value of the metallic parts of the texture
														// Default value 0.5f
	float specular_factor_; 
	std::shared_ptr<LavaImage> opacity_; // If null pick the global opacity mask completely white
	float opacity_mask_;
	std::shared_ptr<LavaImage> normal_; // If null pick the global normal texture
	float use_normal_;	//Determines to use 

	LavaEngine *engine;
};




#endif // !__LAVA_MATERIAL_H__ 
