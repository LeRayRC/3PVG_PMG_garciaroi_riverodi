#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "lava_window.hpp"
#include <iostream>
#include <windows.h>

int main(int argc, char* argv[]) {

// Engine engine;
// engine.init();
// engine.mainLoop();
// engine.cleanUp();

	std::shared_ptr<LavaWindowSystem> a = LavaWindowSystem::Get();
	std::string b = "lava";
	std::unique_ptr<LavaWindow> c =  a->MakeNewWindow(1280, 720, b);

	std::string d = "lava2";
	std::unique_ptr<LavaWindow> e = a->MakeNewWindow(1280, 720, d);


	while (1) {
		
	}
  return 0;
}