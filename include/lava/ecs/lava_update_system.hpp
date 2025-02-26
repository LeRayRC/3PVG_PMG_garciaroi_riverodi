#ifndef __LAVA_UPDATE_SYSTEM_H__
#define __LAVA_UPDATE_SYSTEM_H__ 1

//#include "engine/lava_pipeline.hpp"
#include "lava/ecs/lava_ecs_components.hpp"


class LavaUpdateSystem
{
public:
	LavaUpdateSystem(class LavaEngine& engine);
	~LavaUpdateSystem();


	void update(std::vector<std::optional<UpdateComponent>>&);


private:
	LavaEngine& engine_;
};


#endif // !__LAVA_UPDATE_SYSTEM_H__
