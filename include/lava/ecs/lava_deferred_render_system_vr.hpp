#ifndef __LAVA_DEFERRED_RENDER_SYSTEM_VR_H__
#define __LAVA_DEFERRED_RENDER_SYSTEM_VR_H__ 1

//#include "engine/lava_pipeline.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/ecs/lava_ecs_components.hpp"


class LavaDeferredRenderSystemVR
{
public:
	LavaDeferredRenderSystemVR(class LavaEngineVR& engine);
	~LavaDeferredRenderSystemVR();


	void render(
		uint32_t view_index,
		std::vector<std::optional<TransformComponent>>&,
		std::vector<std::optional<RenderComponent>>&,
		std::vector<std::optional<LightComponent>>& light_component_vector);

private:
	class LavaEngineVR& engine_;

	std::unique_ptr<class LavaPipeline> pipeline_geometry_pass_;
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_first_;
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_additive_;
	std::unique_ptr<class LavaPipeline> pipeline_light_pass_ambient_;
	std::unique_ptr<class LavaPipeline> pipeline_shadows_[3];


	std::shared_ptr<LavaPBRMaterial> light_pass_material;
	std::shared_ptr<class LavaMesh> light_pass_quad_;

	//Gbuffer mapping
	// 0 -> Position 
	// 1 -> Albedo 
	// 2 -> Normal
	static const int gbuffer_count = 3;
	std::shared_ptr<class LavaImage> gbuffers_[gbuffer_count];
	// 0 -> DIRECTIONAL
	// 1 -> POINT 
	// 2 -> SPOT
	std::shared_ptr<class LavaImage> shadowmaps_[3];

	void allocateLights(std::vector<std::optional<struct LightComponent>>& light_component_vector);
	void updateLights(
		uint32_t view_index,
		std::vector<std::optional<struct LightComponent>>& light_component_vector,
		std::vector<std::optional<struct TransformComponent>>& transform_vector);

	void renderGeometryPass(
		uint32_t view_index,
		std::vector<std::optional<TransformComponent>>& transform_vector,
		std::vector<std::optional<RenderComponent>>& render_vector,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderLightPass(
		uint32_t view_index,
		std::vector<std::optional<TransformComponent>>& transform_vector,
		std::vector<std::optional<RenderComponent>>& render_vector,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderShadowMaps(
		uint32_t view_index,
		std::vector<std::optional<TransformComponent>>& transform_vector,
		std::vector<std::optional<RenderComponent>>& render_vector,
		std::vector<std::optional<LightComponent>>& light_component_vector);

	void renderAmbient(uint32_t view_index);

	void setupGBufferBarriers(VkCommandBuffer cmd, VkImageLayout newLayout);
	void setupShadowMapBarriers(VkCommandBuffer cmd, VkImageLayout newLayout);
};


#endif // !__LAVA_NORMAL_RENDER_SYSTEM_H__