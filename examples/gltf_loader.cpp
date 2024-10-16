#include "examples/hellotriangle.hpp"

#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_loader.hpp"
#include "engine/lava_pipeline.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	engine.init();

	LavaPipeline::Config config = {};
	config.device = &engine.device_;
	config.swap_chain = &engine.swap_chain_;
	config.descriptor_set_layout = engine.draw_image_descriptor_set_layout_;
	config.pipeline_flags |= LavaPipeline::PIPELINE_USE_PUSHCONSTANTS;
	config.fragment_shader_path = "../src/shaders/colored_triangle.frag.spv";
	config.vertex_shader_path = "../src/shaders/colored_triangle_mesh.vert.spv";
	


	engine.addPipeline(config);

	engine.mainLoop();

	return 0;
}