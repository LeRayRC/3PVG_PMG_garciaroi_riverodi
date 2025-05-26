#include "lava/window/lava_window_system.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "lava/engine/lava_engine.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_pbr_render_system.hpp"
#include "lava/ecs/lava_update_system.hpp"
#include "lava/ecs/lava_normal_render_system.hpp"
#include "lava/ecs/lava_diffuse_render_system.hpp"
#include "lava/ecs/lava_deferred_render_system.hpp"
#include "lava/common/lava_shapes.hpp"
#include "lava/engine/lava_image.hpp"
#include "imgui.h"


void ecs_render_imgui(LavaECSManager& ecs_manager, size_t camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();
	static bool camera_type = (bool)camera_camera.type_;

	ImGui::Begin("ECS Camera Manager Window");

	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

	}
	if (camera_camera.type_ == CameraType_Perspective) {
		ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
	}
	else {
		ImGui::DragFloat("Size", &camera_camera.size_, 0.01f, 0.0f, 10.0f);

	}
	ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, -360.0f, 360.0f);
	ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);

	ImGui::Checkbox("CameraType", &camera_type); {
		camera_camera.type_ = (CameraType)camera_type;
	}

	ImGui::End();

}

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine(1920, 1080);
	LavaECSManager ecs_manager;
	LavaDiffuseRenderSystem diffuse_render_system{ engine };
	LavaUpdateSystem update_system{ engine };

	std::shared_ptr<LavaPBRMaterial> basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	MeshProperties mesh_properties = {};

	mesh_properties.mesh_path = "../examples/assets/shiba/shiba.glb";
	mesh_properties.material = basic_material;
	std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);





	{
			size_t entity = ecs_manager.createEntity();
			ecs_manager.addComponent<TransformComponent>(entity);
			ecs_manager.addComponent<RenderComponent>(entity);

			auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
			if (transform_component) {
				auto& transform = transform_component->value();
				transform.pos_ = glm::vec3(0, -1.0f, -2.0f);
				transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
			}

			auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
			if (render_component) {
				auto& render = render_component->value();
				render.mesh_ = mesh_;
			}
	}



	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	{

		ecs_manager.addComponent<TransformComponent>(camera_entity);
		ecs_manager.addComponent<CameraComponent>(camera_entity);
		ecs_manager.addComponent<UpdateComponent>(camera_entity);

		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
		camera_tr.rot_ = glm::vec3(-9.0f, 37.0f, 0.0f);
		camera_tr.pos_ = glm::vec3(-1.0f, -0.9f, -1.0f);
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
	}

	engine.global_scene_data_.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

	int count = 0;
	while (!engine.shouldClose()) {




		update_system.update(ecs_manager.getComponentList<UpdateComponent>());

		engine.beginFrame();
		engine.clearWindow();


		diffuse_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());


		ecs_render_imgui(ecs_manager, camera_entity);

		engine.endFrame();
		count++;
	}

	return 0;

}