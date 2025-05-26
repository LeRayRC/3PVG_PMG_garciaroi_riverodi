#include "lava/engine/lava_engine.hpp"
#include "lava/window/lava_window_system.hpp"
#include "lava/window/lava_window.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/ecs/lava_normal_render_system.hpp"
#include "lava/ecs/lava_diffuse_render_system.hpp"
#include "lava/jobsystem/lava_job_system.hpp"
#include <imgui.h>


#include "thread"


std::filesystem::path meshes[3]{"../examples/assets/shiba/shiba.glb", "../examples/assets/skull.glb", "../examples/assets/shark.glb" };
int mesh_index = 0;
std::queue<std::future<std::shared_ptr<LavaMesh>>> mesh_queue;

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
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{ engine };
	LavaDiffuseRenderSystem diffuse_render_system{ engine };

	LavaJobSystem job_system;

	///////////////////////
	//////ASSETS START/////
	///////////////////////
	std::shared_ptr<LavaPBRMaterial> basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	
	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = meshes[0];
	mesh_properties.material = basic_material;

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
		transform.pos_ = glm::vec3(0.0f, 0.0f, -1.0f);
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
	camera_tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	LavaInput* input = engine.window_.get_input();

	engine.setMainCamera(&camera_camera, &camera_tr);

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

		ecs_render_imgui(ecs_manager, (int)camera_entity);

		//normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
		//	ecs_manager.getComponentList<RenderComponent>());

		diffuse_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}