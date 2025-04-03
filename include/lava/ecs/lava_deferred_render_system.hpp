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
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_first_;
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_additive_;
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_ambient_;
	std::unique_ptr<class LavaPipeline> pipeline_shadows_[3];


	LavaPBRMaterial light_pass_material;
	std::shared_ptr<class LavaMesh> light_pass_quad_;


	//Gbuffer mapping
	// 0 -> Position 
	// 1 -> Albedo 
	// 2 -> Normal
	static const int gbuffer_count = 3;
	std::shared_ptr<LavaImage> gbuffers_[gbuffer_count];
	//DIRECTIONAL, POINT , SPOT
	std::shared_ptr<LavaImage> shadowmaps_[3];

	//AllocatedImage shadowmap_image_[3];
	//VkSampler shadowmap_sampler_[3];

	void allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector);
	void update_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector,
		std::vector<std::optional<struct TransformComponent>>& transform_vector);

	void renderGeometryPass(std::vector<std::optional<TransformComponent>>& transform_vector,
		std::vector<std::optional<RenderComponent>>& render_vector,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderLightPass(std::vector<std::optional<TransformComponent>>& transform_vector,
		std::vector<std::optional<RenderComponent>>& render_vector,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderShadowMaps(std::vector<std::optional<TransformComponent>>& transform_vector,
		std::vector<std::optional<RenderComponent>>& render_vector,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderAmbient();

	void setupGBufferBarriers(VkCommandBuffer cmd, VkImageLayout newLayout);
	void setupShadowMapBarriers(VkCommandBuffer cmd, VkImageLayout newLayout);
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__