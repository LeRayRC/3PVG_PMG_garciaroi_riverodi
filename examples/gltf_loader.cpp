#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	engine.init();

	MaterialProperties mat_properties = {};
	mat_properties.name = "Basic Material";
	mat_properties.vertex_shader_path		= "../src/shaders/colored_triangle_mesh.vert.spv";
	mat_properties.fragment_shader_path = "../src/shaders/colored_triangle.frag.spv";
	mat_properties.pipeline_flags |= PipelineFlags::PIPELINE_USE_PUSHCONSTANTS;

	LavaMaterial basic_material(engine, mat_properties);

	uint32_t  black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	uint32_t  yellow = glm::packUnorm4x8(glm::vec4(1, 1, 0, 1));
	std::array<uint32_t, 16 * 16> pixels;
	for (int x = 0; x<16; x++) {
		for (int y = 0; y < 16; y++) {
			pixels[x + y*16] = ((x % 2) ^ (y % 2)) ? yellow : black;
		}
	}
	LavaImage checker_board_image = LavaImage(&engine, pixels.data(), VkExtent3D{ 16, 16, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/shiba/shiba.glb";
	mesh_properties.material = &basic_material;

	std::shared_ptr<LavaMesh> mesh = engine.addMesh(mesh_properties);



	engine.mainLoop();

	return 0;
}