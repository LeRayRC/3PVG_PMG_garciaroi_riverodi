/**
 * @file lava_input.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Input's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "lava_input.hpp"
#include "lava_engine.hpp"

std::unordered_map<GLFWwindow*, LavaInput*> LavaInput::input_map;

LavaInput::LavaInput(GLFWwindow* window) : window_{window}
{
	glfwSetKeyCallback(window_, global_key_callback);
	glfwSetMouseButtonCallback(window_, global_mouse_callback);
	glfwSetWindowCloseCallback(window_, clean_bindings);

	if(input_map.empty()) LavaEngine::end_frame_callbacks.push_back(end_frame);

	input_map.emplace(window_, this);
}

LavaInput::~LavaInput()
{
	clean_bindings(window_);
}

bool LavaInput::isInputPressed(int key)
{
	return kb_mouse_input_properties_map[key].past_frame_properties & KEY_PRESS;
}

bool LavaInput::isInputReleased(int key)
{
	return kb_mouse_input_properties_map[key].past_frame_properties & KEY_RELEASE;
}

bool LavaInput::isInputUp(int key)
{
	return kb_mouse_input_properties_map[key].past_frame_properties == 0 ||
		kb_mouse_input_properties_map[key].past_frame_properties & KEY_RELEASE;
}

bool LavaInput::isInputDown(int key)
{
	return kb_mouse_input_properties_map[key].past_frame_properties & KEY_PRESS ||
		kb_mouse_input_properties_map[key].past_frame_properties & KEY_REPEAT;
}

bool LavaInput::isActionPressed(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputPressed(i)) return true;
	}

	for (GamePadAction i : gamepad_action_map[action]) {
		if (isGamePadButtonPressed(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

bool LavaInput::isActionReleased(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputReleased(i)) return true;
	}

	for (GamePadAction i : gamepad_action_map[action]) {
		if (isGamePadButtonReleased(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

bool LavaInput::isActionUp(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputUp(i)) return true;
	}

	for (GamePadAction i : gamepad_action_map[action]) {
		if (isGamePadButtonUp(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

bool LavaInput::isActionDown(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputDown(i)) return true;
	}

	for (GamePadAction i : gamepad_action_map[action]) {
		if (isGamePadButtonDown(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

float LavaInput::getActionAxis(int action)
{
	return getGamePadAxis(gamepad_axis_action_map[action].game_pad, gamepad_axis_action_map[action].game_pad_button);
}

void LavaInput::BindActionToInput(int action, int key)
{
	//Find or Create the new Action
	kb_mouse_action_map[action].insert(key);
}

bool LavaInput::isGamePadButtonPressed(int game_pad, int button)
{
	return game_pad_states[game_pad].state.buttons[button] & KEY_PRESS && 
		   game_pad_states[game_pad].is_active;
}

bool LavaInput::isGamePadButtonReleased(int game_pad, int button)
{
	return game_pad_states[game_pad].state.buttons[button] & KEY_RELEASE &&
		   game_pad_states[game_pad].is_active;
}

bool LavaInput::isGamePadButtonUp(int game_pad, int button)
{
	return (game_pad_states[game_pad].state.buttons[button] == 0 ||
		    game_pad_states[game_pad].state.buttons[button] & KEY_RELEASE) &&
		    game_pad_states[game_pad].is_active;
}

bool LavaInput::isGamePadButtonDown(int game_pad, int button)
{
	return (game_pad_states[game_pad].state.buttons[button] & KEY_PRESS ||
		    game_pad_states[game_pad].state.buttons[button] & KEY_REPEAT) &&
		    game_pad_states[game_pad].is_active;
}

float LavaInput::getGamePadAxis(int game_pad, int axis)
{
	if (!game_pad_states[game_pad].is_active) return 0;
	return game_pad_states[game_pad].state.axes[axis];
}

void LavaInput::BindActionToGamePadButton(int action, int game_pad, int button)
{
	//Find or Create the new Action
	gamepad_action_map[action].push_back(GamePadAction(game_pad, button));
}

void LavaInput::BindActionToGamePadAxis(int action, int game_pad, int axis)
{
	//Find or Create the new Action
	gamepad_axis_action_map[action].game_pad = game_pad;
	gamepad_axis_action_map[action].game_pad_button = axis;
}

void LavaInput::kb_mouse_callback(int key, int scancode, int action, int mods)
{
	int32_t aux_current_frame_properties = 0;
	if (action & GLFW_PRESS) {
		aux_current_frame_properties = aux_current_frame_properties | KEY_PRESS;
		if(scancode == -1) aux_current_frame_properties = aux_current_frame_properties | SUSTAIN_TIL_RELEASE;
	}
	else if (action & GLFW_REPEAT) aux_current_frame_properties = aux_current_frame_properties | KEY_REPEAT;
	else aux_current_frame_properties = aux_current_frame_properties | KEY_RELEASE;
	//Find or Create the new key
	kb_mouse_input_properties_map[key].current_frame_properties = aux_current_frame_properties;
	kb_mouse_input_properties_map[key].last_action_time = glfwGetTime();
}

void LavaInput::ProcessEndFrame()
{
	for(auto i = kb_mouse_input_properties_map.begin(); i != kb_mouse_input_properties_map.end(); i++) {
		i->second.past_frame_properties = i->second.current_frame_properties;
		if (i->second.current_frame_properties & SUSTAIN_TIL_RELEASE) i->second.current_frame_properties = SUSTAIN_TIL_RELEASE | KEY_REPEAT;
		else i->second.current_frame_properties = 0;
	}

	for (auto i = game_pad_states.begin(); i != game_pad_states.end(); i++) {
		GLFWgamepadstate aux = i->second.state;
		if (glfwGetGamepadState(i->first, &i->second.state) == GLFW_TRUE) {
			i->second.is_active = true;
			for (int j = 0; j < 15; j++) {
				if (aux.buttons[j] == GLFW_RELEASE && i->second.state.buttons[j] == GLFW_PRESS) {
					i->second.state.buttons[j] = KEY_PRESS;
				}
				else if (aux.buttons[j] == KEY_PRESS || aux.buttons[j] == KEY_REPEAT) {
					if(i->second.state.buttons[j] == GLFW_PRESS) i->second.state.buttons[j] = KEY_REPEAT;
					else i->second.state.buttons[j] = KEY_RELEASE;
				}
			}
		}
		else i->second.is_active = false;
	}
}

void LavaInput::global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	input_map[window]->kb_mouse_callback(key, scancode, action, mods);
}

void LavaInput::global_mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	input_map[window]->kb_mouse_callback(button, -1, action, mods);
}

void LavaInput::clean_bindings(GLFWwindow* window)
{
	glfwSetKeyCallback(window, nullptr);
	input_map.erase(window);
}

void LavaInput::end_frame()
{
	for (std::pair<GLFWwindow*, LavaInput*> input_pair : input_map) {
		input_pair.second->ProcessEndFrame();
	}
}