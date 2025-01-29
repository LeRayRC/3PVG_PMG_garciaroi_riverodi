#ifndef __LAVA_PBR_RENDER_SYSTEM_H__
#define __LAVA_PBR_RENDER_SYSTEM_H__ 1

#include "engine/lava_pipeline.hpp"
#include "ecs/lava_ecs_components.hpp"


class LavaPBRRenderSystem
{
public:
	LavaPBRRenderSystem(class LavaEngine& engine);
	~LavaPBRRenderSystem();


	void render(std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&,
		std::vector<std::optional<LightComponent>>& light_component_vector);

private:
	class LavaEngine& engine_;
	LavaPipeline pipeline_;
	LavaPipeline pipeline_first_light_;


	std::unique_ptr<LavaBuffer> pbr_data_buffer_;
	std::unique_ptr<LavaBuffer> light_data_buffer_;
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__
