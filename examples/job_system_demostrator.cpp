#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_material.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_normal_render_system.hpp"
#include "lava_job_system.hpp"
#include "imgui.h"

#include "thread"


std::filesystem::path meshes[3]{"../examples/assets/shiba/shiba.glb", "../examples/assets/skull.glb", "../examples/assets/shark.glb" };
int mesh_index = 0;
std::queue<std::future<std::shared_ptr<LavaMesh>>> mesh_queue;

void ecs_render_imgui(LavaECSManager& ecs_manager, int camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	ImGui::Begin("ECS Manager Window");



	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

	}

	ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
	ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, 88.0f, 268.0f);
	ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);

	ImGui::End();

	camera_camera.LookAt(camera_tr.pos_, camera_tr.rot_);

}



int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{ engine };
	LavaJobSystem job_system;

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
	mesh_properties.mesh_path = meshes[0];
	mesh_properties.material = &basic_material;

	std::shared_ptr<LavaMesh> mesh_loaded = std::make_shared<LavaMesh>(engine, mesh_properties);

	/////////////////////
	//////ASSETS END/////
	/////////////////////

	//auto mesh_loaded = job_system.add([&engine, mesh_properties]() { return std::make_shared<LavaMesh>(engine, mesh_properties); });

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
		render.mesh_ = mesh_loaded ;
	}

	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);

	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(180.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 10.0f);
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	LavaInput* input = engine.window_.get_input();

	while (!engine.shouldClose()) {

		if (input->isInputReleased(KEY_RIGHT)) {
			mesh_index++;
			mesh_index = mesh_index % 3;
			mesh_properties.mesh_path = meshes[mesh_index];
			mesh_queue.push(job_system.add([&engine, mesh_properties]() { return std::make_shared<LavaMesh>(engine, mesh_properties); }));
		}
		else if (input->isInputReleased(KEY_LEFT)) {
			mesh_index += 4;
			mesh_index = mesh_index % 3;
			mesh_properties.mesh_path = meshes[mesh_index];
			mesh_queue.push(job_system.add([&engine, mesh_properties]() { return std::make_shared<LavaMesh>(engine, mesh_properties); }));
		}

		if (!mesh_queue.empty()) {
			auto status = mesh_queue.front().wait_for(std::chrono::seconds(0));
			if (status == std::future_status::ready) {
				render_component = ecs_manager.getComponent<RenderComponent>(entity);
				auto& render = render_component->value();
				render.mesh_ = mesh_queue.front().get();
				mesh_queue.pop();
			}
		}

		engine.global_scene_data_.view = camera_camera.view_;
		engine.global_scene_data_.proj = glm::perspective(glm::radians(camera_camera.fov_),
			(float)engine.window_extent_.width / (float)engine.window_extent_.height, camera_camera.near_, camera_camera.far_);
		engine.global_scene_data_.proj[1][1] *= -1;
		engine.global_scene_data_.viewproj = engine.global_scene_data_.proj * engine.global_scene_data_.view;
		engine.global_data_buffer_->updateBufferData(&engine.global_scene_data_, sizeof(GlobalSceneData));

		engine.beginFrame();
		engine.clearWindow();

		//engine.renderImgui();
		ecs_render_imgui(ecs_manager, camera_entity);

		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}