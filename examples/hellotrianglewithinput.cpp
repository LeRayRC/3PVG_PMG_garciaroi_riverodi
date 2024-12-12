#include "examples/hellotriangle.hpp"

#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_normal_render_system.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaNormalRenderSystem normal_render_system{engine};
	
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
		transform.pos_ = glm::vec3(0.0f, 0.0f, -10.0f);
		transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);

	}
	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	if (render_component) {
		auto& render = render_component->value();
		render.mesh_ = mesh_triangle;
	}

	LavaInput* input = engine.window_.get_input();

	while (!engine.shouldClose()) {

		engine.beginFrame();
		engine.clearWindow();
		glm::vec2 mouse_pos = input->getMousePosition();

		// Movement
		float ndcX = (2.0f * mouse_pos.x) / (float)engine.window_extent_.width - 1.0f;
		float ndcY = (2.0f * mouse_pos.y) / (float)engine.window_extent_.height - 1.0f;
		auto dir = glm::vec3(glm::inverse(engine.global_scene_data_.viewproj) * glm::vec4(ndcX, ndcY, 1.0f, 1.0f));
		dir *= -(transform_component->value().pos_.z);
		transform_component->value().pos_ = dir;

		// Rotation
		float rot_act = 0.0f;
		if(input->isInputDown(KEY_RIGHT)) rot_act = -10000.0f ;
		else if (input->isInputDown(KEY_LEFT)) rot_act = 10000.0f;
		transform_component->value().rot_.z += (rot_act * engine.dt_);
		
		//Scale
		glm::vec2 scrooll = input->getScrollOffset();
		transform_component->value().scale_ += (engine.dt_ * scrooll.y * 100.0f);

		engine.renderImgui();
		normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>());
		engine.endFrame();
	}

	return 0;
}