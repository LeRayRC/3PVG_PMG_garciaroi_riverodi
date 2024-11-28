#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_material.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_normal_render_system.hpp"
#include "imgui.h"


void ecs_render_imgui(LavaECSManager& ecs_manager, int camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();
	
	ImGui::Begin("ECS Example Manager Window");


	
	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {
		
	}

	if(ImGui::DragFloat3("Camera rot", &camera_tr.rot_.x, 0.5f, -180.0f, 180.0f)) {
	}
	
	ImGui::End();


	//camera_camera.setViewYXZ(camera_tr.pos_, camera_tr.rot_);
	camera_camera.LookAt(camera_tr.pos_, camera_tr.rot_);
	
}


int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{engine};

	///////////////////////
	//////ASSETS START/////
	///////////////////////
	MaterialProperties mat_properties = {};
	mat_properties.name = "Basic Material";
	mat_properties.vertex_shader_path = "../src/shaders/normal.vert.spv";
	mat_properties.fragment_shader_path = "../src/shaders/normal.frag.spv";
	mat_properties.pipeline_flags = PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET;

	LavaMaterial basic_material(engine, mat_properties);
	std::vector<Vertex> triangle_vertices(3);

	triangle_vertices[0].position = { 0.5,0.5, 0 };
	triangle_vertices[1].position = { 0.0,-0.5, 0 };
	triangle_vertices[2].position = { -0.5,0.5, 0 };

	triangle_vertices[0].normal = { 0.0,0.0, 1.0f };
	triangle_vertices[1].normal = { 0.0,0.0, 1.0f };
	triangle_vertices[2].normal = { 0.0,0.0, 1.0f };

	triangle_vertices[0].color = { 0,0, 0,1 };
	triangle_vertices[1].color = { 0.5,0.5,0.5 ,1 };
	triangle_vertices[2].color = { 1,0, 0,1 };

	std::vector<uint32_t> triangle_index(3);
	triangle_index[0] = 0;
	triangle_index[1] = 1;
	triangle_index[2] = 2;

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/shiba/shiba_test.glb";
	mesh_properties.material = &basic_material;
	//mesh_properties.index = triangle_index;
	//mesh_properties.vertex = triangle_vertices;

	std::shared_ptr<LavaMesh> mesh_shiba = std::make_shared<LavaMesh>(engine, mesh_properties);

	/////////////////////
	//////ASSETS END/////
	/////////////////////
	int width = 30;

#ifndef NDEBUG
	for (int x = 0; x < 30; x++) {
		for (int z = 0; z < 30; z++) {

			size_t entity = ecs_manager.createEntity();
			ecs_manager.addComponent<TransformComponent>(entity);
			ecs_manager.addComponent<RenderComponent>(entity);

			auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
			if (transform_component) {
				auto& transform = transform_component->value();
				transform.pos_ = glm::vec3(-15 + x, -5.0f, 5.0f + 2.0f*z);
				transform.rot_ = glm::vec3(-90.0f, 0.0f, 0.0f);
				transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);

			}
			auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
			if (render_component) {
				auto& render = render_component->value();
				render.mesh_ = mesh_shiba;
			}
}
	}
#else
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 100; y++) {

			size_t entity = ecs_manager.createEntity();
			ecs_manager.addComponent<TransformComponent>(entity);
			ecs_manager.addComponent<RenderComponent>(entity);

			auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
			if (transform_component) {
				auto& transform = transform_component->value();
				transform.pos_ = glm::vec3(-500.0f + x, 50.0f - y, -15.0f);
				transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);

			}
			auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
			if (render_component) {
				auto& render = render_component->value();
				render.mesh_ = mesh_shiba;
			}
		}
}
#endif

	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);

	

	
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(180.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 70.0f);
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();



		
	


	while (!engine.shouldClose()) {


		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(0)->value();
		//camera_tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
		camera_tr.pos_ = glm::vec3(camera_tr.pos_.x+0.02f, -5.0f, 0.0f);
		//for (auto& comp : ecs_manager.getComponentList<TransformComponent>()) {
		//	if (comp) {
		//		auto& transform = comp.value();
		//		transform.rot_ = glm::vec3(transform.rot_.x,0.03f * engine.frame_data_.frame_number_,
		//			0.0f);
		//	}
		//}

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

		
		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}