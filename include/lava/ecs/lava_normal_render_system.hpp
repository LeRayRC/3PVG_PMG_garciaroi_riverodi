#ifndef __LAVA_NORMAL_RENDER_SYSTEM_H__
#define __LAVA_NORMAL_RENDER_SYSTEM_H__ 1

#include "lava/ecs/lava_ecs_components.hpp"


class LavaNormalRenderSystem
{
public:
	LavaNormalRenderSystem(class LavaEngine& engine);
	~LavaNormalRenderSystem();


	void render(std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&);

private:
	class LavaEngine& engine_;
	std::unique_ptr<class LavaPipeline> pipeline_;
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__
