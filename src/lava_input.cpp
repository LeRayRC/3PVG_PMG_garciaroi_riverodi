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
	glfwSetJoystickCallback(global_joystick_callback);
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
	return false;
}

bool LavaInput::isActionReleased(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputReleased(i)) return true;
	}
	return false;
}

bool LavaInput::isActionUp(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputUp(i)) return true;
	}
	return false;
}

bool LavaInput::isActionDown(int action)
{
	for (int i : kb_mouse_action_map[action]) {
		if (isInputDown(i)) return true;
	}
	return false;
}

void LavaInput::BindActionToInput(int action, int key)
{
	//Find or Create the new Action
	kb_mouse_action_map[action].insert(key);
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
}

void LavaInput::global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	input_map[window]->kb_mouse_callback(key, scancode, action, mods);
}

void LavaInput::global_mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	input_map[window]->kb_mouse_callback(button, -1, action, mods);
}

void LavaInput::global_joystick_callback(int jid, int event)
{
	printf("\n %d \n", jid);
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