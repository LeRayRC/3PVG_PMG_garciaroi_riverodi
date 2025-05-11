#ifndef __LAVA_DIFFUSE_RENDER_SYSTEM_VR_H__
#define __LAVA_DIFFUSE_RENDER_SYSTEM_VR_H__ 1

#include "lava/ecs/lava_ecs_components.hpp"


class LavaDiffuseRenderSystemVR
{
public:
	LavaDiffuseRenderSystemVR(class LavaEngineVR& engine);
	~LavaDiffuseRenderSystemVR();


	void render(uint32_t view_index,
		std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&);

private:
	class LavaEngineVR& engine_;
	std::unique_ptr<class LavaPipeline> pipeline_;
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__
