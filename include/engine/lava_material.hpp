#ifndef __LAVA_MATERIAL_H__ 
#define __LAVA_MATERIAL_H__ 1

#include "lava_types.hpp"
#include "lava_engine.hpp"

struct LavaMaterialImage {
	bool active;
	class LavaImage* diffuse;
	class LavaImage* normal;

	LavaMaterialImage() {
		active = false;
		diffuse = nullptr;
		normal = nullptr;
	}
	LavaMaterialImage(bool active, class LavaImage* diffuse, class LavaImage* normal) {
		this->active = active;
		this->diffuse = diffuse;
		this->normal = normal;
	}

};

class LavaMaterial
{
public:
	LavaMaterial(class LavaEngine& engine, MaterialProperties prop);
	~LavaMaterial();

	std::string get_name() { return name_; }
	
	LavaPipeline& get_pipeline() { return pipeline_; }

	static const unsigned int material_images_count = 1;
	LavaMaterialImage get_image(unsigned int index);
	void set_image(unsigned int index, LavaMaterialImage image); 
	VkDescriptorSet get_descriptor_set() {
		return descriptor_set_;
	}
private:
	std::string name_;
	LavaPipeline pipeline_;
	VkDescriptorSet descriptor_set_;
	LavaDescriptorManager& descriptor_manager_;
	std::array<LavaMaterialImage, material_images_count > images_;
	LavaEngine *engine;
};




#endif // !__LAVA_MATERIAL_H__ 
