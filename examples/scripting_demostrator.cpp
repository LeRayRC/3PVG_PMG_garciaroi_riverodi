
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
#include "lava/common/lava_world.hpp"
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
	LavaWorld& world = LavaWorld::GetWorld();

	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{engine};

	world.setECSManager(&ecs_manager);

	///////////////////////
	//////ASSETS START/////
	///////////////////////
	std::shared_ptr<LavaPBRMaterial> basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/shiba/shiba.glb";
	mesh_properties.material = basic_material;

	std::shared_ptr<LavaMesh> mesh_shiba = std::make_shared<LavaMesh>(engine, mesh_properties);

	/////////////////////
	//////ASSETS END/////
	/////////////////////


	for (int i = 0; i < 10; i++) {
		size_t scripted_entity = ecs_manager.createEntity();
		ecs_manager.addComponent<LuaScriptComponent>(scripted_entity);
		auto script_component = ecs_manager.getComponent<LuaScriptComponent>(scripted_entity);
		if (script_component) {
			if (script_component->has_value()) {
				auto& script_component_value = script_component->value();
				script_component_value.lua_script_path = "../examples/scripts/test_ecs.lua";
				script_component_value.set_entity(scripted_entity);
				script_component_value.script_->run(script_component_value.lua_script_path);
			}
		}

		auto transform_component = ecs_manager.getComponent<TransformComponent>(scripted_entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(-4 + i * 0.5f, 0.0f, -1.0f);
			transform.rot_ = glm::vec3(-90.0f, 0.0f, 0.0f);
			transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
		}
		auto render_component = ecs_manager.getComponent<RenderComponent>(scripted_entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_shiba;
		}
	}

	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);

	
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	engine.setMainCamera(&camera_camera, &camera_tr);


	while (!engine.shouldClose()) {


		engine.beginFrame();
		engine.clearWindow();

		engine.renderImgui();
		ecs_render_imgui(ecs_manager, camera_entity);

		
		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}