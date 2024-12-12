
#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_material.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_normal_render_system.hpp"
#include "imgui.h"
#include "scripting/lava_lua_script.hpp"
#include "lava_world.hpp"





void ecs_render_imgui(LavaECSManager& ecs_manager, size_t camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();
	
	ImGui::Begin("ECS Example Manager Window");


	
	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {
		
	}

	ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
	ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, 88.0f, 268.0f);
	ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);
	
	ImGui::End();

	camera_camera.LookAt(camera_tr.pos_, camera_tr.rot_);
	
}

void engine_imgui_window(LavaEngine& engine) {
	ImGui::Begin("Engine Window Manager");

	ImGui::Text("Frame time: %f", engine.dt_);
	ImGui::Text("FPS: %f", 1 / engine.dt_);

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


	for (int i = 0; i < 10; i++) {
		size_t scripted_entity = ecs_manager.createEntity();
		ecs_manager.addComponent<LuaScriptComponent>(scripted_entity);
		auto script_component = ecs_manager.getComponent<LuaScriptComponent>(scripted_entity);
		if (script_component) {
			if (script_component->has_value()) {
				auto& script_component_value = script_component->value();
				script_component_value.lua_script_path = "../examples/scripts/test_ecs.lua";
				script_component_value.script_->run(script_component_value.lua_script_path);
			}
		}

		auto transform_component = ecs_manager.getComponent<TransformComponent>(scripted_entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(-8 + i, -5.0f, 5.0f);
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
	camera_tr.rot_ = glm::vec3(180.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 15.0f);
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();


	while (!engine.shouldClose()) {

		engine.global_scene_data_.view = camera_camera.view_;
		engine.global_scene_data_.proj = glm::perspective(glm::radians(camera_camera.fov_),
			(float)engine.window_extent_.width / (float)engine.window_extent_.height, camera_camera.near_, camera_camera.far_);
		engine.global_scene_data_.proj[1][1] *= -1;
		engine.global_scene_data_.viewproj = engine.global_scene_data_.proj * engine.global_scene_data_.view;
		engine.global_data_buffer_->updateBufferData(&engine.global_scene_data_, sizeof(GlobalSceneData));

		engine.beginFrame();
		engine.clearWindow();
		


		engine.renderImgui();
		ecs_render_imgui(ecs_manager, camera_entity);
		engine_imgui_window(engine);

		
		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}