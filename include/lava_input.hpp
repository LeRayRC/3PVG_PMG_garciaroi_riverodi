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

	// Keys & Mouse Implementation

	bool isInputPressed(int key);

	bool isInputReleased(int key);

	bool isInputUp(int key);

	bool isInputDown(int key);

	void BindActionToInput(int action, int key);

	// Gamepad Implementation
		  //TO DO//

	// Action Implementation

	bool isActionPressed(int action);

	bool isActionReleased(int action);

	bool isActionUp(int action);

	bool isActionDown(int action);

private:

	LavaInput() = delete;

	GLFWwindow* window_;

	// Keys & Mouse Implementation
	std::unordered_map<int, KeyProperties> kb_mouse_input_properties_map;
	std::map<int, std::set<int>> kb_mouse_action_map;
	void kb_mouse_callback(int key, int scancode, int action, int mods);

	// GamePad Implementation
	GLFWgamepadstate game_pad_state;

	void ProcessEndFrame();

	//Static Properties and Functions

	static std::unordered_map<GLFWwindow*, LavaInput*> input_map;

	static void global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void global_mouse_callback(GLFWwindow* window, int button, int action, int mods);

	static void global_joystick_callback(int jid, int event);

	static void clean_bindings(GLFWwindow* window);

	static void end_frame();
};

#endif