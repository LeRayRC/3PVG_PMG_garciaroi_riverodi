
#include "lava/engine/lava_engine.hpp"
#include "lava/window/lava_window_system.hpp"
#include "lava/window/lava_window.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/ecs/lava_normal_render_system.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{engine};
	
	LavaPBRMaterial basic_material(engine, MaterialPBRProperties());

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
	mesh_properties.name = "Triangle Mesh";
	mesh_properties.type = MESH_CUSTOM;
	mesh_properties.material = &basic_material;
	mesh_properties.index = triangle_index;
	mesh_properties.vertex = triangle_vertices;

	std::shared_ptr<LavaMesh> mesh_triangle = std::make_shared<LavaMesh>(engine, mesh_properties);



	size_t entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(entity);
	ecs_manager.addComponent<RenderComponent>(entity);

	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	if (transform_component) {
		auto& transform = transform_component->value();
		transform.pos_ = glm::vec3(0.0f, 0.0f, -15.0f);
		transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);

	}
	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	if (render_component) {
		auto& render = render_component->value();
		render.mesh_ = mesh_triangle;
	}


	size_t camera_entity = ecs_manager.createEntity();
	{

		ecs_manager.addComponent<TransformComponent>(camera_entity);
		ecs_manager.addComponent<CameraComponent>(camera_entity);

		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
		camera_tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
		camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
		auto& camera_component = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();
		engine.setMainCamera(&camera_component, &camera_tr);
	}
	engine.global_scene_data_.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);


	while (!engine.shouldClose()) {


		engine.beginFrame();
		engine.clearWindow();

		engine.renderImgui();
		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}