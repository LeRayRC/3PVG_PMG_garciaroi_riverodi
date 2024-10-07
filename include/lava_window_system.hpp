/**
 * @file lava_window_system.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Window Sytem's header file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#ifndef  __LAVA_WINDOW_SYSTEM_
#define  __LAVA_WINDOW_SYSTEM_ 1

#include "custom_types.hpp"

class LavaWindowSystem {
public:
	static std::shared_ptr<LavaWindowSystem> Get() {
		if (!is_initialized) {
			instance_ = std::make_shared<LavaWindowSystem>();
			is_initialized = true;
		}
		return instance_;
	}

	std::unique_ptr<class LavaWindow> MakeNewWindow(int x, int y, std::string& name);

	~LavaWindowSystem() {
		glfwTerminate();
	}

	//SHOULD BE PRIVATE
private:

	static bool is_initialized;

	static std::shared_ptr<LavaWindowSystem> instance_;

	LavaWindowSystem() { glfwInit(); }

	//NEVER USE
	LavaWindowSystem(const LavaWindowSystem&) = delete; // CONSTRUCTOR DE COPIA
	LavaWindowSystem& operator=(const LavaWindowSystem&) = delete; // ASIGNACION DE COPIA
	LavaWindowSystem(LavaWindowSystem&&) = delete; // CONSTRUCTOR DE MOVIMIENTO
	LavaWindowSystem& operator=(LavaWindowSystem&&) = delete; // ASIGNACION DE MOVIMIENTO

	friend std::shared_ptr<LavaWindowSystem> std::make_shared<LavaWindowSystem>();
};

#endif // ! __LAVA_WINDOW_SYSTEM_