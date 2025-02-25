/** 
 * @page how_to_hello_world Hello World
 * @ingroup how_to
 *
 * @section section_hello_world_simple_window Your first window
 * 
 * With this code you can open a simple window within the engine
 * 
 * @code{.cpp}
 * #include "lava/engine/lava_engine.hpp"
 * #include "lava/window/lava_window_system.hpp"
 * #include "lava/window/lava_window.hpp"
 * 
 * int main(int argc, char* argv[]) {
 * 	std::shared_ptr<LavaWindowSystem> window_system = LavaWindowSystem::Get();
 * 	LavaEngine engine;
 * 	while (!engine.shouldClose()) {
 * 		engine.pollEvents();
 * 	}
 * 
 *   return 0;
 * }
 * @endcode
 * 
 * 
 * This code just creates an engine instance which has a window associated directly
 */
