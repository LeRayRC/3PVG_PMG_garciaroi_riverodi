#include "examples/hellotriangle.hpp"

#include "lava_engine.hpp"
#include "lava_window_system.hpp"

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	engine.init();
	engine.mainLoop();

	return 0; 
}