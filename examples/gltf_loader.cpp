#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_material.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_normal_render_system.hpp"
#include "imgui.h"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{ engine };

	///////////////////////
	//////ASSETS START/////
	///////////////////////
	MaterialProperties mat_properties = {};
	mat_properties.name = "Basic Material";
	mat_properties.vertex_shader_path = "../src/shaders/normal.vert.spv";
	mat_properties.fragment_shader_path = "../src/shaders/normal.frag.spv";
	mat_properties.pipeline_flags = PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET;

	LavaMaterial basic_material(engine, mat_properties);

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/shiba/shiba.glb";
	mesh_properties.material = &basic_material;

	std::shared_ptr<LavaMesh> mesh_shiba = std::make_shared<LavaMesh>(engine, mesh_properties);

	/////////////////////
	//////ASSETS END/////
	/////////////////////


	size_t entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(entity);
	ecs_manager.addComponent<RenderComponent>(entity);

	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	if (transform_component) {
		auto& transform = transform_component->value();
		transform.pos_ = glm::vec3(0.0f, 0.0f, -5.0f);
		transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);

	}
	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	if (render_component) {
		auto& render = render_component->value();
		render.mesh_ = mesh_shiba;
	}



	while (!engine.shouldClose()) {

		for (auto& comp : ecs_manager.getComponentList<TransformComponent>()) {
			if (comp) {
				auto& transform = comp.value();
				transform.rot_ = glm::vec3(0.001f * engine.frame_data_.frame_number_,
					0.002f * engine.frame_data_.frame_number_,
					0.0005f * engine.frame_data_.frame_number_);
			}
		}


		engine.beginFrame();
		engine.clearWindow();

		engine.renderImgui();
		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}