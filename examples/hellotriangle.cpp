#include "examples/hellotriangle.hpp"

#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	engine.init();

	MaterialProperties mat_properties = {};
	mat_properties.name = "Basic Material";
	mat_properties.vertex_shader_path = "../src/shaders/colored_triangle_mesh.vert.spv";
	mat_properties.fragment_shader_path = "../src/shaders/colored_triangle.frag.spv";
	mat_properties.pipeline_flags |= PipelineFlags::PIPELINE_USE_PUSHCONSTANTS;

	LavaMaterial basic_material(engine, mat_properties);

	std::vector<Vertex> triangle_vertices(3);

	triangle_vertices[0].position = { 0.5,0.5, 0 };
	triangle_vertices[1].position = { 0.0,-0.5, 0 };
	triangle_vertices[2].position = { -0.5,0.5, 0 };

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
	mesh_properties.mesh_path = "../examples/assets/shiba/shiba.glb";
	mesh_properties.material = &basic_material;
	mesh_properties.vertex = triangle_vertices;
	mesh_properties.index = triangle_index;

	std::shared_ptr<LavaMesh> mesh = engine.addMesh(mesh_properties);

	engine.mainLoop();

	return 0;
}