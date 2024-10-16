/**
 * @file lava_input.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Input's header file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef  __LAVA_INPUT_
#define  __LAVA_INPUT_ 1

#include "lava_types.hpp"

class LavaInput {

public:

	LavaInput(GLFWwindow* window);

	~LavaInput();

	bool isKeyPressed(int key);

	bool isKeyReleased(int key);

	bool isKeyUp(int key);

	bool isKeyDown(int key);

private:

	LavaInput() = delete;

	GLFWwindow* window_;

	std::unordered_map<int, KeyProperties> input_properties_map;

	void key_callback(int key, int scancode, int action, int mods);

	void ProcessEndFrame();

	//Static Properties and Functions

	static std::unordered_map<GLFWwindow*, LavaInput*> input_map;

	static void global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void clean_bindings(GLFWwindow* window);

	static void end_frame();
};

#endif