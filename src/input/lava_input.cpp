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

#include "lava/input/lava_input.hpp"
#include "lava/engine/lava_engine.hpp"

std::unordered_map<GLFWwindow*, LavaInput*> LavaInput::s_input_map_;

LavaInput::LavaInput(GLFWwindow* window) : window_{window}
{
	//Set all callbacks to the window
	glfwSetKeyCallback(window_, global_key_callback);
	glfwSetMouseButtonCallback(window_, global_mouse_callback);
	glfwSetWindowCloseCallback(window_, clean_bindings);
	glfwSetCursorPosCallback(window_, global_cursor_position_callback);
	glfwSetScrollCallback(window_, global_scrooll_callback);

	//If this is the first window add the end frame callback to the engine
	if(s_input_map_.empty()) LavaEngine::end_frame_callbacks.push_back(end_frame);

	//Add this window to the map
	s_input_map_.emplace(window_, this);
	cursor_pos_ = glm::vec2(0,0);
	scroll_offset_ = glm::vec2(0,0);
	next_scroll_offset_ = glm::vec2(0,0);
}

LavaInput::~LavaInput()
{
	//Clear all bindings to the window
	clean_bindings(window_);
}

bool LavaInput::isInputPressed(int key)
{
	return kb_mouse_input_properties_map_[key].past_frame_properties & KEY_PRESS;
}

bool LavaInput::isInputReleased(int key)
{
	return kb_mouse_input_properties_map_[key].past_frame_properties & KEY_RELEASE;
}

bool LavaInput::isInputUp(int key)
{
	return kb_mouse_input_properties_map_[key].past_frame_properties == 0 ||
		kb_mouse_input_properties_map_[key].past_frame_properties & KEY_RELEASE;
}

bool LavaInput::isInputDown(int key)
{
	return kb_mouse_input_properties_map_[key].past_frame_properties & KEY_PRESS ||
		kb_mouse_input_properties_map_[key].past_frame_properties & KEY_REPEAT;
}

glm::vec2 LavaInput::getMousePosition()
{
	return cursor_pos_;
}

glm::vec2 LavaInput::getScrollOffset()
{
	return scroll_offset_;
}



bool LavaInput::isActionPressed(int action)
{
	//Check all input(keyboard and mouse) if one is true return true
	for (int i : kb_mouse_action_map_[action]) {
		if (isInputPressed(i)) return true;
	}

	//Check all input(gamepad) if one is true return true
	for (GamePadAction i : gamepad_action_map_[action]) {
		if (isGamePadButtonPressed(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

bool LavaInput::isActionReleased(int action)
{
	//Check all input(keyboard and mouse) if one is true return true
	for (int i : kb_mouse_action_map_[action]) {
		if (isInputReleased(i)) return true;
	}
	//Check all input(gamepad) if one is true return true
	for (GamePadAction i : gamepad_action_map_[action]) {
		if (isGamePadButtonReleased(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

bool LavaInput::isActionUp(int action)
{
	//Check all input(keyboard and mouse) if one is false return false(at least one is down)
	for (int i : kb_mouse_action_map_[action]) {
		if (isInputDown(i)) return false;
	}

	//Check all input(gamepad) if one is false return false(at least one is down)
	for (GamePadAction i : gamepad_action_map_[action]) {
		if (isGamePadButtonDown(i.game_pad, i.game_pad_button)) return false;
	}

	return true;
}

bool LavaInput::isActionDown(int action)
{
	//Check all input(keyboard and mouse) if one is true return true(at least one is down)
	for (int i : kb_mouse_action_map_[action]) {
		if (isInputDown(i)) return true;
	}

	//Check all input(gamepad) if one is true return true(at least one is down)
	for (GamePadAction i : gamepad_action_map_[action]) {
		if (isGamePadButtonDown(i.game_pad, i.game_pad_button)) return true;
	}

	return false;
}

float LavaInput::getActionAxis(int action)
{
	return getGamePadAxis(gamepad_axis_action_map_[action].game_pad, gamepad_axis_action_map_[action].game_pad_button);
}

void LavaInput::BindActionToInput(int action, int key)
{
	//Find or Create the new Action
	kb_mouse_action_map_[action].insert(key);
}

bool LavaInput::isGamePadButtonPressed(int game_pad, int button)
{
	return game_pad_states_[game_pad].state.buttons[button] & KEY_PRESS &&
		game_pad_states_[game_pad].is_active;
}

bool LavaInput::isGamePadButtonReleased(int game_pad, int button)
{
	return game_pad_states_[game_pad].state.buttons[button] & KEY_RELEASE &&
		game_pad_states_[game_pad].is_active;
}

bool LavaInput::isGamePadButtonUp(int game_pad, int button)
{
	return (game_pad_states_[game_pad].state.buttons[button] == 0 ||
		game_pad_states_[game_pad].state.buttons[button] & KEY_RELEASE) &&
		game_pad_states_[game_pad].is_active;
}

bool LavaInput::isGamePadButtonDown(int game_pad, int button)
{
	return (game_pad_states_[game_pad].state.buttons[button] & KEY_PRESS ||
		game_pad_states_[game_pad].state.buttons[button] & KEY_REPEAT) &&
		game_pad_states_[game_pad].is_active;
}

float LavaInput::getGamePadAxis(int game_pad, int axis)
{
	if (!game_pad_states_[game_pad].is_active) return 0;
	return game_pad_states_[game_pad].state.axes[axis];
}

void LavaInput::BindActionToGamePadButton(int action, int game_pad, int button)
{
	//Find or Create the new Action
	gamepad_action_map_[action].push_back(GamePadAction(game_pad, button));
}

void LavaInput::BindActionToGamePadAxis(int action, int game_pad, int axis)
{
	//Find or Create the new Action
	gamepad_axis_action_map_[action].game_pad = game_pad;
	gamepad_axis_action_map_[action].game_pad_button = axis;
}

void LavaInput::kb_mouse_callback(int key, int input_type, int action, int mods)
{
	int32_t aux_current_frame_properties = 0;
	if (action & GLFW_PRESS) { //If is press this frame
		aux_current_frame_properties = aux_current_frame_properties | KEY_PRESS | SUSTAIN_TIL_RELEASE; //Add Press property
		//If input type is mouse add Sustain Til Release Property
		/*if (input_type == -1) aux_current_frame_properties = aux_current_frame_properties | SUSTAIN_TIL_RELEASE;*/
	}
	else if (action & GLFW_REPEAT) aux_current_frame_properties = aux_current_frame_properties | KEY_REPEAT | SUSTAIN_TIL_RELEASE; //If is repeated this frame add Repeated property
	else aux_current_frame_properties = aux_current_frame_properties | KEY_RELEASE; //If not Press or Repeated then is Release, add Release property
	//Find or Create the new key and set current frame property
	kb_mouse_input_properties_map_[key].current_frame_properties = aux_current_frame_properties;
	//Set current time(not currently use)
	kb_mouse_input_properties_map_[key].last_action_time = glfwGetTime();
}

void LavaInput::cursor_position_callback(double xpos, double ypos)
{
	cursor_pos_.x = (float)xpos;
	cursor_pos_.y = (float)ypos;
}

void LavaInput::scrooll_callback(double xoffset, double yoffset)
{
	next_scroll_offset_.x = (float)xoffset;
	next_scroll_offset_.y = (float)yoffset;
}

void LavaInput::ProcessEndFrame()
{
	//Process al keys in kb_mouse_input_properties_map_(keyboard and mouse)
	for (auto i = kb_mouse_input_properties_map_.begin(); i != kb_mouse_input_properties_map_.end(); i++) {
		//Assign the data collected this frame in the past frame
		i->second.past_frame_properties = i->second.current_frame_properties;
		// If posses Sustanin Til Release Property assing Sustain Til Release and Repeat property to the new frame property
		if (i->second.current_frame_properties & SUSTAIN_TIL_RELEASE) i->second.current_frame_properties = SUSTAIN_TIL_RELEASE | KEY_REPEAT;
		else i->second.current_frame_properties = 0; // Else assign no properties
	}

	//Process al gamepad states in game_pad_states_(only gamepads)
	for (auto i = game_pad_states_.begin(); i != game_pad_states_.end(); i++) {
		//Save past frame state for later checking
		GLFWgamepadstate aux = i->second.state;
		if (glfwGetGamepadState(i->first, &i->second.state) == GLFW_TRUE) { // If gamepad is connected process the state
			i->second.is_active = true;
			for (int j = 0; j < 15; j++) {	//Process all buttons
				if (aux.buttons[j] == 0 && i->second.state.buttons[j] == GLFW_PRESS) { // If past state is 0 and current is Press set Press property
					i->second.state.buttons[j] = KEY_PRESS;
				}
				else if (aux.buttons[j] == KEY_PRESS || aux.buttons[j] == KEY_REPEAT) {	// If past state is PRESS or REPEAT
					if(i->second.state.buttons[j] == GLFW_PRESS) i->second.state.buttons[j] = KEY_REPEAT;	// If current state is press set Repeat property
					else i->second.state.buttons[j] = KEY_RELEASE;	// else(curren state is Release) set current state as release
				}
			}
		}
		else i->second.is_active = false;  // If gamepad is not conected set is_active to false
	}

	//Scrooll offset back to cero
	scroll_offset_ = next_scroll_offset_;
	next_scroll_offset_ = { 0.0f, 0.0f };
}

void LavaInput::global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	s_input_map_[window]->kb_mouse_callback(key, scancode, action, mods);
}

void LavaInput::global_mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	s_input_map_[window]->kb_mouse_callback(button, -1, action, mods);
}

void LavaInput::global_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	s_input_map_[window]->cursor_position_callback(xpos, ypos);
}

void LavaInput::global_scrooll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	s_input_map_[window]->scrooll_callback(xoffset, yoffset);
}

void LavaInput::clean_bindings(GLFWwindow* window)
{
	glfwSetKeyCallback(window, nullptr);
	s_input_map_.erase(window);
}

void LavaInput::end_frame()
{
	for (std::pair<GLFWwindow*, LavaInput*> input_pair : s_input_map_) {
		input_pair.second->ProcessEndFrame();
	}
}