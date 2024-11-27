/**
 * @file lava_window.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Window's header file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#ifndef  __LAVA_WINDOW_
#define  __LAVA_WINDOW_ 1

#include "lava_types.hpp"
#include "lava_input.hpp"


class LavaWindow {
public:

	LavaWindow(LavaWindow&& other);

	~LavaWindow();

	//Should be private
	LavaWindow(GLFWwindow* w) : w_{ w } { window_input_ = std::make_unique<LavaInput>(w_); }

	LavaWindow(unsigned int x, unsigned int y, const char* name);

	GLFWwindow* get_window() const;

	LavaInput* get_input() const;

	//TALK ABOUT THIS WITH DANI
	static std::unique_ptr<LavaWindow> make(unsigned int x, unsigned int y, std::string& name) {
		GLFWwindow* w = glfwCreateWindow(x, y, name.c_str(), nullptr, nullptr);
		if (nullptr == w) return nullptr;
		return  std::make_unique<LavaWindow>(w);//LavaWindow{ w };
	}

private:

	GLFWwindow* w_;

	std::unique_ptr<LavaInput> window_input_;

	LavaWindow() { w_ = nullptr; }

	LavaWindow(const LavaWindow&) = delete;
	LavaWindow& operator=(const LavaWindow&) = delete;

	friend class LavaWindowSystem;
	//friend std::unique_ptr<LavaWindow> std::make_unique<LavaWindow>(GLFWwindow&);
};


#endif // ! __LAVA_WINDOW_