#include "lava/engine/lava_engine.hpp"
#include "lava/window/lava_window_system.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_update_system.hpp"
#include "lava/ecs/lava_gaussian_render_system.hpp"
#include "lava/ecs/lava_diffuse_render_system.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/engine/lava_image.hpp"
#include "lava/common/lava_shapes.hpp"
#include "imgui.h"
#include "lava/gaussian/lava_gaussian_splat.hpp"

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
	LavaEngine engine(1280, 720);
	LavaECSManager ecs_manager;
	LavaGaussianRenderSystem pbr_render_system{ engine };
	LavaDiffuseRenderSystem diffuse_render_system{ engine };
	LavaUpdateSystem update_system{ engine };
	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);
	ecs_manager.addComponent<UpdateComponent>(camera_entity);

	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(-25.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 6.0f, 6.0f);
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

	engine.global_scene_data_.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

	LavaGaussianSplat gaussian_splat;
	TransformComponent gaussian_trans;
	gaussian_splat.importPly(&engine, "../examples/assets/lilly_boquet.ply");
	{
		while (!engine.shouldClose()) {


			update_system.update(ecs_manager.getComponentList<UpdateComponent>());

			engine.beginFrame();
			engine.clearWindow();

			pbr_render_system.render(gaussian_trans,
				gaussian_splat);



			ecs_render_imgui(ecs_manager, camera_entity);

			engine.endFrame();
		}

		return 0;
	}
}