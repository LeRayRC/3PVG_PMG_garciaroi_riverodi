#include "lava/engine/lava_engine.hpp"
#include "lava/window/lava_window_system.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_update_system.hpp"
#include "lava/ecs/lava_pbr_render_system.hpp"
#include "lava/ecs/lava_diffuse_render_system.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/engine/lava_image.hpp"
#include "lava/common/lava_shapes.hpp"
#include "imgui.h"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaPBRRenderSystem pbr_render_system{ engine };
	LavaUpdateSystem update_system{ engine };

	pbr_render_system.setClearValue(148.0f/255.0f, 140.0f/255.0f, 116.0f/255.0f);


	{
		MaterialPBRProperties mat_properties = {};
		mat_properties.name = "PBR Material";

		std::shared_ptr<LavaPBRMaterial> basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());

		MeshProperties mesh_properties = {};
		mesh_properties.name = "Shiba Mesh";
		mesh_properties.mesh_path = "../examples/assets/regirock/scene.gltf";
		mesh_properties.material = basic_material;

		std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);

		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
			transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}


	{
		size_t light_entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_SPOT;
			light.base_color_ = glm::vec3(190.0f/255.0f, 229.0f/255.0f, 214.0f/255.0f);
			light.spec_color_ = glm::vec3(1.0f, 1.0f, 1.0f);
			light.cutoff_ = 8.0f;
			light.outer_cutoff_ = 10.5f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_ = glm::vec3(-90.0f, 0.0f, 10.5f);
			tr.pos_ = glm::vec3(-7.230f, 30.0f, 0.0f);
		}

	}


	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);
	ecs_manager.addComponent<UpdateComponent>(camera_entity);

	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(-60.0f, -30.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(4.3f, 14.0f, 10.0f);
	auto& camera_component = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	auto update_component = ecs_manager.getComponent<UpdateComponent>(camera_entity);
	if (update_component) {
		auto& update = update_component->value();

		update.id = camera_entity;
		update.ecs_manager = &ecs_manager;
		update.update_ = [](size_t id, LavaECSManager* ecs_manager, LavaEngine& engine) {
			UpdateCameraWithInput(id, ecs_manager, engine);
			};
	}

	engine.setMainCamera(&camera_component, &camera_tr);

	engine.global_scene_data_.ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
	while (!engine.shouldClose()) {

		update_system.update(ecs_manager.getComponentList<UpdateComponent>());
		engine.beginFrame();
		engine.clearWindow();
		pbr_render_system.renderWithShadows(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());
		engine.endFrame();
	}

	return 0;
}