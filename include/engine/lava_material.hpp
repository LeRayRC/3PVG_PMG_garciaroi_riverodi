#ifndef __LAVA_MATERIAL_H__ 
#define __LAVA_MATERIAL_H__ 1

#include "lava_types.hpp"
#include "lava_engine.hpp"

class LavaMaterial
{
public:
	LavaMaterial(class LavaEngine& engine, MaterialProperties prop);
	~LavaMaterial();

	std::string get_name() { return name_; }
	
	LavaPipeline& get_pipeline() { return pipeline_; }

private:
	std::string name_;
	LavaPipeline pipeline_;
};




#endif // !__LAVA_MATERIAL_H__ 
