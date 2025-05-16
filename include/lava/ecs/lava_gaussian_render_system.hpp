#ifndef __LAVA_GAUSSIAN_RENDER_SYSTEM_H__
#define __LAVA_GAUSSIAN_RENDER_SYSTEM_H__ 1

#include "lava/ecs/lava_ecs_components.hpp"

class LavaGaussianRenderSystem
{
public:
	LavaGaussianRenderSystem(class LavaEngine& engine);
	~LavaGaussianRenderSystem();


	void render(TransformComponent&,
		class LavaGaussianSplat&);

private:
	class LavaEngine& engine_;
	std::unique_ptr<class LavaPipeline> pipeline_;
};


#endif // !__LAVA_GAUSSIAN_RENDER_SYSTEM_H__
