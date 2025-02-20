/**
 * @file lava_window_system.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Window System's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "lava/window/lava_window_system.hpp"
#include "lava/window/lava_window.hpp"

bool LavaWindowSystem::is_initialized = false;
std::shared_ptr<LavaWindowSystem> LavaWindowSystem::instance_ = nullptr;

std::unique_ptr<class LavaWindow> LavaWindowSystem::MakeNewWindow(int x, int y, std::string& name)
{
	return LavaWindow::make(x, y, name);
}