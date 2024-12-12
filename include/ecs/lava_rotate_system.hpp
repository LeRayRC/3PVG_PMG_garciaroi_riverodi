#ifndef __LAVA_ROTATE_SYSTEM_H__
#define __LAVA_ROTATE_SYSTEM_H__ 1

#include "ecs/lava_ecs_components.hpp"

class LavaRotateSystem
{
public:
	LavaRotateSystem();
	~LavaRotateSystem();

	void operator()(std::vector<std::optional<RotateComponent>>&, 
		std::vector<std::optional<TransformComponent>>& transform_component_vector, const float dt) const;
		
private:

};



#endif // !__LAVA_ROTATE_SYSTEM_H_
