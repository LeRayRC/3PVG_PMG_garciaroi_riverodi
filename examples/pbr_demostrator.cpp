#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_pbr_material.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_pbr_render_system.hpp"
#include "imgui.h"


void ecs_render_imgui(LavaECSManager& ecs_manager, size_t camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	ImGui::Begin("ECS Camera Manager Window");

	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

	}

	ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
	ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, 88.0f, 268.0f);
	ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);

	ImGui::End();

	
}

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaPBRRenderSystem pbr_render_system{ engine };

	///////////////////////
	//////ASSETS START/////
	///////////////////////
	MaterialPBRProperties mat_properties = {};
	mat_properties.name = "PBR Material";
	mat_properties.pipeline_flags = PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET;

	LavaPBRMaterial basic_material(engine, mat_properties);

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/Avocado.glb";
	mesh_properties.material = &basic_material;

	std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);

	/////////////////////
	//////ASSETS END/////
	/////////////////////


	size_t entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(entity);
	ecs_manager.addComponent<RenderComponent>(entity);

	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	if (transform_component) {
		auto& transform = transform_component->value();
		transform.pos_ = glm::vec3(0.0f, 0.0f, -1.0f);
		transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
	}
	

	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	if (render_component) {
		auto& render = render_component->value();
		render.mesh_ = mesh_;
	}

	size_t light_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(light_entity);
	ecs_manager.addComponent<LightComponent>(light_entity);
	
		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_DIRECTIONAL;
			light.base_color_ = glm::vec3(1.0f, 1.0f, 0.0f);
			light.spec_color_ = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		
	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);

	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(180.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
	auto& camera_component = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	engine.global_scene_data_.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

	LavaInput* input = engine.window_.get_input();

	while (!engine.shouldClose()) {

		//auto &tr = ecs_manager.getComponent<TransformComponent>(entity)->value();
		//tr.rot_ = glm::vec3(0.1f * engine.frame_data_.frame_number_,
		//	0.02f * engine.frame_data_.frame_number_,
		//	0.05f * engine.frame_data_.frame_number_);
		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();

		//if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

		//}
		//ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
		//ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, 88.0f, 268.0f);
		//ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);

		engine.updateMainCamera(&camera_component, &camera_tr);

		if (input->isInputDown(KEY_D)) {
			camera_tr.pos_.x += (1.0f * engine.dt_);
		}
		if (input->isInputDown(KEY_A)) {
			camera_tr.pos_.x -= (1.0f * engine.dt_);
		}
		if (input->isInputDown(KEY_W)) {
			camera_tr.pos_.x += (1.0f * engine.dt_);
		}
		if (input->isInputDown(KEY_W)) {
			camera_tr.pos_.x -= (1.0f * engine.dt_);
		}


		engine.beginFrame();
		engine.clearWindow();

		engine.renderImgui();
		ecs_render_imgui(ecs_manager, camera_entity);

		pbr_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());
		//normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
		//	ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}