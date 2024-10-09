#ifndef __LAVA_INSTANCE_H_
#define __LAVA_INSTANCE_H_ 1 

#include "lava_types.hpp"

class LavaInstance
{
public:
	LavaInstance(std::vector<const char*> validation_layers);
	~LavaInstance();

	VkInstance get_instance() const;
private:
	VkInstance instance_;

	LavaInstance(const LavaInstance& obj) = delete;
};


#endif
