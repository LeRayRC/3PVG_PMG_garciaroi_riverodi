#ifndef __LAVA_DEFERRED_RENDER_SYSTEM_H__
#define __LAVA_DEFERRED_RENDER_SYSTEM_H__ 1

//#include "engine/lava_pipeline.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/ecs/lava_ecs_components.hpp"


class LavaDeferredRenderSystem
{
public:
	LavaDeferredRenderSystem(class LavaEngine& engine);
	~LavaDeferredRenderSystem();


	void render(std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	//void renderWithShadows(std::vector<std::optional<TransformComponent>>&,
	//	std::vector<std::optional<RenderComponent>>&,
	//	std::vector<std::optional<LightComponent>>& light_component_vector);

private:
	class LavaEngine& engine_;
	std::unique_ptr<class LavaPipeline> pipeline_geometry_pass_;
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_;
	std::unique_ptr<class LavaPipeline> pipeline_first_light_;
	std::unique_ptr<class LavaPipeline> pipeline_shadows_[3];


	LavaPBRMaterial light_pass_material;
	std::shared_ptr<class LavaMesh> light_pass_quad_;


	//std::unique_ptr<class LavaImage> shadowmap_image_;
	//Shadow Maps Images
	// 0 -> Position 
	// 1 -> Albedo 
	// 2 -> Normal
	static const int gbuffer_count = 3;
	std::shared_ptr<LavaImage> gbuffers_[gbuffer_count];
	//AllocatedImage gbuffer_[gbuffer_count];
	//VkSampler gbuffer_sampler_[gbuffer_count];


	AllocatedImage shadowmap_image_[3];
	VkSampler shadowmap_sampler_[3];


	void allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector);
	void update_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector,
		std::vector<std::optional<struct TransformComponent>>& transform_vector);

	void setupGBufferBarriers(VkCommandBuffer cmd, VkImageLayout newLayout);
	void setupShadowMapBarriers(VkCommandBuffer cmd, VkImageLayout newLayout);

	void pipelineBarrierForRenderPassStart(VkCommandBuffer cmd);
	void pipelineBarrierForRenderPassEnd(VkCommandBuffer cmd);
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__