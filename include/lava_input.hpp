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
	bool isGamePadButtonPressed(int game_pad, int button);

	bool isGamePadButtonReleased(int game_pad, int button);

	bool isGamePadButtonUp(int game_pad, int button);

	bool isGamePadButtonDown(int game_pad, int button);

	float getGamePadAxis(int game_pad, int axis);

	void BindActionToGamePadButton(int action, int game_pad, int button);

	void BindActionToGamePadAxis(int action, int game_pad, int axis);

	// Action Implementation

	bool isActionPressed(int action);

	bool isActionReleased(int action);

	bool isActionUp(int action);

	bool isActionDown(int action);

	float getActionAxis(int action);

private:

	LavaInput() = delete;

	GLFWwindow* window_;

	// Keys & Mouse Implementation
	std::unordered_map<int, KeyProperties> kb_mouse_input_properties_map;
	std::map<int, std::set<int>> kb_mouse_action_map;
	void kb_mouse_callback(int key, int scancode, int action, int mods);

	// GamePad Implementation
	std::map<int, GamepadState> game_pad_states;
	std::map<int, std::vector<GamePadAction>> gamepad_action_map;
	std::map<int, GamePadAction> gamepad_axis_action_map;

	void ProcessEndFrame();

	//Static Properties and Functions

	static std::unordered_map<GLFWwindow*, LavaInput*> input_map;

	static void global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void global_mouse_callback(GLFWwindow* window, int button, int action, int mods);

	static void clean_bindings(GLFWwindow* window);

	static void end_frame();
};

#endif