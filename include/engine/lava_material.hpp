#ifndef __LAVA_MATERIAL_H__ 
#define __LAVA_MATERIAL_H__ 1

#include "lava_pipeline.hpp"

class LavaMaterial
{
public:
	LavaMaterial(LavaPipeline::Config config);
	~LavaMaterial();

	struct Properties {
		std::string name;
	};

	LavaPipeline& get_pipeline() { return pipeline_; }

private:
	LavaPipeline pipeline_;
};




#endif // !__LAVA_MATERIAL_H__ 
