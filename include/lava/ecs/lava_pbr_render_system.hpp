#ifndef __LAVA_PBR_RENDER_SYSTEM_H__
#define __LAVA_PBR_RENDER_SYSTEM_H__ 1

//#include "engine/lava_pipeline.hpp"
#include "lava/ecs/lava_ecs_components.hpp"


class LavaPBRRenderSystem
{
public:
	LavaPBRRenderSystem(class LavaEngine& engine);
	~LavaPBRRenderSystem();


	void render(std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderWithShadows(std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&,
		std::vector<std::optional<LightComponent>>& light_component_vector);

private:
	class LavaEngine& engine_;
	std::unique_ptr<class LavaPipeline> pipeline_;
	std::unique_ptr<class LavaPipeline> pipeline_first_light_;
	std::unique_ptr<class LavaPipeline> pipeline_shadows_;

	//Shadow Maps Images
	AllocatedImage shadowmap_image_;
	VkSampler shadowmap_sampler_;

	void allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector);
	void update_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector,
		std::vector<std::optional<struct TransformComponent>>& transform_vector);
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__
