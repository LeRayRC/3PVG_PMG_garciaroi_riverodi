#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "lava_window.hpp"
#include <iostream>
#include <windows.h>

int main(int argc, char* argv[]) {

	std::shared_ptr<LavaWindowSystem> window_system = LavaWindowSystem::Get();

	//Simplest way to open a window
	std::string window_name = "lava test window";
	std::unique_ptr<LavaWindow> window = window_system->MakeNewWindow(1280, 720, window_name);

	Sleep(5000);

  return 0;
}