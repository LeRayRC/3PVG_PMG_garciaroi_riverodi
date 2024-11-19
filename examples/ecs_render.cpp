#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_material.hpp"
#include "ecs/lava_ecs.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	
	size_t entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(entity);

	auto component = ecs_manager.getComponent<TransformComponent>(entity);
	if (component) {
		auto& transform = component->value();
		printf("Entity %d\n Pos(%f,%f,%f)\n Rot(%f,%f,%f)\n Scale(%f,%f,%f)\n",
			entity,
			transform.pos_.x, transform.pos_.y, transform.pos_.z,
			transform.rot_.x, transform.rot_.y, transform.rot_.z,
			transform.scale_.x, transform.scale_.y, transform.scale_.z); 
	}

	ecs_manager.removeComponent<TransformComponent>(entity);



	MaterialProperties mat_properties = {};
	mat_properties.name = "Basic Material";
	mat_properties.vertex_shader_path = "../src/shaders/colored_triangle_mesh.vert.spv";
	mat_properties.fragment_shader_path = "../src/shaders/colored_triangle.frag.spv";
	mat_properties.pipeline_flags = PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET;

	LavaMaterial basic_material(engine, mat_properties);

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/shiba/shiba.glb";
	mesh_properties.material = &basic_material;

	std::shared_ptr<LavaMesh> mesh = engine.addMesh(mesh_properties);
	mesh->get_transform().set_pos(glm::vec3(0.0f, 0.0f, -10.0f));
	mesh->get_transform().set_scale(glm::vec3(5.0f, 5.0f, 5.0f));

	engine.mainLoop();

	return 0;
}