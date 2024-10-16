#include "lava_input.hpp"
#include "lava_input.hpp"
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
	glfwSetWindowCloseCallback(window_, clean_bindings);

	if(input_map.empty()) LavaEngine::end_frame_callbacks.push_back(end_frame);

	input_map.emplace(window_, this);
}

LavaInput::~LavaInput()
{
	clean_bindings(window_);
}

bool LavaInput::isKeyPressed(int key)
{
	return input_properties_map[key].past_frame_properties & KEY_PRESS;
}

bool LavaInput::isKeyReleased(int key)
{
	return input_properties_map[key].past_frame_properties & KEY_RELEASE;
}

bool LavaInput::isKeyUp(int key)
{
	return input_properties_map[key].past_frame_properties == 0 || 
		   input_properties_map[key].past_frame_properties & KEY_RELEASE;
}

bool LavaInput::isKeyDown(int key)
{
	return input_properties_map[key].past_frame_properties & KEY_PRESS ||
		input_properties_map[key].past_frame_properties & KEY_REPEAT;
}

void LavaInput::key_callback(int key, int scancode, int action, int mods)
{
	int32_t aux_current_frame_properties = 0;
	if (action & GLFW_PRESS) {
		aux_current_frame_properties = aux_current_frame_properties | KEY_PRESS;
	}
	else if (action & GLFW_REPEAT) aux_current_frame_properties = aux_current_frame_properties | KEY_REPEAT;
	else aux_current_frame_properties = aux_current_frame_properties | KEY_RELEASE;

	//Find or Create the new key
	input_properties_map[key].current_frame_properties = aux_current_frame_properties;
	input_properties_map[key].last_action_time = glfwGetTime();
}

void LavaInput::ProcessEndFrame()
{
	for(auto i = input_properties_map.begin(); i != input_properties_map.end(); i++) {
		i->second.past_frame_properties = i->second.current_frame_properties;
		i->second.current_frame_properties = 0;
	}
}

void LavaInput::global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	input_map[window]->key_callback(key, scancode, action, mods);
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