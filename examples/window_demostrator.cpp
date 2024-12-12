#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "lava_window.hpp"
#include <iostream>
//#include <windows.h>

int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem> window_system = LavaWindowSystem::Get();
	LavaEngine engine;

	//Simplest way to open a window
	//std::string window_name = "lava test window";
	//std::unique_ptr<LavaWindow> window = window_system->MakeNewWindow(1280, 720, window_name);
	//
	while (!engine.shouldClose()) {
		//engine.beginFrame();
		//engine.clearWindow();
		//engine.endFrame();
		engine.pollEvents();
	}

  return 0;
}