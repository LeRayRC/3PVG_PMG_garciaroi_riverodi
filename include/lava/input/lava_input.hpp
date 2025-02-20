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

 //Key Actions
#define KEY_PRESS 0x0001
#define KEY_RELEASE 0x0002
#define KEY_REPEAT 0x0004
#define SUSTAIN_TIL_RELEASE 0x0008

//Redefine Keys for easy use(can use glfw key defines for those not included)
#define KEY_SPACE 32
#define KEY_ESCAPE 256
#define KEY_ENTER 257
#define KEY_TAB 258
#define KEY_BACKSPACE 259
#define KEY_RIGHT 262
#define KEY_LEFT 263
#define KEY_DOWN 264
#define KEY_UP 265
#define KEY_A   65
#define KEY_B   66
#define KEY_C   67
#define KEY_D   68
#define KEY_E   69
#define KEY_F   70
#define KEY_G   71
#define KEY_H   72
#define KEY_I   73
#define KEY_J   74
#define KEY_K   75
#define KEY_L   76
#define KEY_M   77
#define KEY_N   78
#define KEY_O   79
#define KEY_P   80
#define KEY_Q   81
#define KEY_R   82
#define KEY_S   83
#define KEY_T   84
#define KEY_U   85
#define KEY_V   86
#define KEY_W   87
#define KEY_X   88
#define KEY_Y   89
#define KEY_Z   90


//Redefine Mouse Buttons for easy use(can use glfw key defines for those not included)
#define 	MOUSE_BUTTON_1   0
#define 	MOUSE_BUTTON_2   1
#define 	MOUSE_BUTTON_3   2

struct KeyProperties {
	KeyProperties() {
		current_frame_properties = 0;
		past_frame_properties = 0;
		last_action_time = 0;
	}
	int32_t current_frame_properties;
	int32_t past_frame_properties;

	double last_action_time;
};

typedef struct GamepadStateWrapper {
	GamepadStateWrapper() {
		for (int i = 0; i < 6; i++) {
			state.axes[i] = 0;
			state.buttons[i] = 0;
		}
		for (int i = 6; i < 15; i++) {
			state.buttons[i] = 0;
		}
		is_active = false;
	}
	GLFWgamepadstate state;
	bool is_active;
}GamepadState;

typedef struct GamePadActionWrapper {
	GamePadActionWrapper(int game_pad_, int game_pad_button_) {
		game_pad = game_pad_;
		game_pad_button = game_pad_button_;
	}
	GamePadActionWrapper() {
		game_pad = 0;
		game_pad_button = 0;
	}
	int game_pad;
	int game_pad_button;
}GamePadAction;


class LavaInput {

public:

	/**
	* @brief Default constructor
	*
	* @param window the window to link this input.
	*/
	LavaInput(GLFWwindow* window);

	/**
	* @brief Default destructor
	*
	*/
	~LavaInput();

	/****************** Keys & Mouse Implementation **************/

	/**
	* @brief Check the first frame the key is being press. (ONLY FOR KEYBOARD AND MOUSE)
	* 
	* @param key the key to be check
	*/
	bool isInputPressed(int key);

	/**
	* @brief Check the first frame the key is releases. (ONLY FOR KEYBOARD AND MOUSE)
	*
	* @param key the key to be check
	*/
	bool isInputReleased(int key);

	/**
	* @brief Check if the key is not being press. (ONLY FOR KEYBOARD AND MOUSE)
	*
	* @param key the key to be check
	*/
	bool isInputUp(int key);

	/**
	* @brief Check if the key is being press. (ONLY FOR KEYBOARD AND MOUSE)
	*
	* @param key the key to be check
	*/
	bool isInputDown(int key);

	/*
	* @brief Return the mouse position in a vector(glm:vec2)
	*/
	glm::vec2 getMousePosition();

	/*
	* @brief Return the scroll offset in a vector(glm:vec2)
	*/
	glm::vec2 getScrollOffset();

	/**
	* @brief Bind the recieve action to a key from mouse or keyboard. (ONLY FOR KEYBOARD AND MOUSE)
	*
	* @param action the action to bind(Should be an enum define by the user)
	* @param key the key to be bound
	*/
	void BindActionToInput(int action, int key);



	/****************** Gamepad Implementation **************/

	/**
	* @brief Check the first frame the button, in the specified gamepad, is being press. (ONLY FOR GAMEPAD)
	*
	* @param game_pad the gamepad to be check
	* @param button the button to be check
	*/
	bool isGamePadButtonPressed(int game_pad, int button);

	/**
	* @brief Check the first frame the button, in the specified gamepad, is released. (ONLY FOR GAMEPAD)
	*
	* @param game_pad the gamepad to be check
	* @param button the button to be check
	*/
	bool isGamePadButtonReleased(int game_pad, int button);

	/**
	* @brief Check if the button, in the specified gamepad, is not being press. (ONLY FOR GAMEPAD)
	*
	* @param game_pad the gamepad to be check
	* @param button the button to be check
	*/
	bool isGamePadButtonUp(int game_pad, int button);

	/**
	* @brief Check if the button, in the specified gamepad, is being press. (ONLY FOR GAMEPAD)
	*
	* @param game_pad the gamepad to be check
	* @param button the button to be check
	*/
	bool isGamePadButtonDown(int game_pad, int button);

	/**
	* @brief Return the value of the axis in the specified gamepad, value between -1.0 and 1.0. (RANGE: [-1.0f, 1.0f]) (ONLY FOR GAMEPAD)
	*
	* @param game_pad the gamepad to be check
	* @param button the button to be check
	*/
	float getGamePadAxis(int game_pad, int axis);

	/**
	* @brief Bind the recieve action to a button from the specified gamepad. (ONLY FOR GAMEPAD)
	*
	* @param action the action to bind(Should be an enum define by the user)
	* @param game_pad the gamepad to be bound
	* @param button the button to be bound
	*/
	void BindActionToGamePadButton(int action, int game_pad, int button);

	/**
	* @brief Bind the recieve action to an axis from the specified gamepad, an action can only be bound to one axis. (ONLY FOR GAMEPAD)
	*
	* @param action the action to bind(Should be an enum define by the user)
	* @param game_pad the gamepad to be bound
	* @param axis the axis to be bound
	*/
	void BindActionToGamePadAxis(int action, int game_pad, int axis);



	/****************** Action Implementation **************/

	/**
	* @brief Check the first frame the action is being press in all it's binds(keyboard, mouse or gamepad),
	*		 return true if at least one of them is being press for the first time. (ONLY FOR ALREADY BIND ACTIONS)
	*
	* @param action the action to be check(Should be an enum define by the user)
	*/
	bool isActionPressed(int action);

	/**
	* @brief Check the first frame the action is released in all it's binds(keyboard, mouse or gamepad),
	*		 return true if at least one of them is being release this frame. (ONLY FOR ALREADY BIND ACTIONS)
	*
	* @param action the action to be check(Should be an enum define by the user)
	*/
	bool isActionReleased(int action);

	/**
	* @brief Check if the action is not press in all it's binds(keyboard, mouse or gamepad),
	*		 return true if all of them are not being press. (ONLY FOR ALREADY BIND ACTIONS)
	*
	* @param action the action to be check(Should be an enum define by the user)
	*/
	bool isActionUp(int action);

	/**
	* @brief Check if the action is press in all it's binds(keyboard, mouse or gamepad),
	*		 return true if at least one of them is being press. (ONLY FOR ALREADY BIND ACTIONS)
	*
	* @param action the action to be check(Should be an enum define by the user)
	*/
	bool isActionDown(int action);

	/**
	* @brief Return the value of the action, value between -1.0 and 1.0. (RANGE: [-1.0f, 1.0f]) (ONLY FOR ALREADY BIND ACTIONS)
	*
	* @param action the action to be check(Should be an enum define by the user)
	*/
	float getActionAxis(int action);

private:

	LavaInput() = delete;

	/**
	* @var GLFWwindow window_
	* @brief Stores the glfw window link to the input.
	*/
	GLFWwindow* window_;

	/**
	* @brief Process the value of all keys and collect the state of all use gamepads.
	*/
	void ProcessEndFrame();

	/**
	* @brief Cursor position updated to the curren value
	*/
	glm::vec2 cursor_pos_;

	/**
	* @brief Scroll Offset updated to the curren value
	*/
	glm::vec2 scroll_offset_;

	/**
	* @brief Scroll Offset updated to the next frame value
	*/
	glm::vec2 next_scroll_offset_;

	/****************** Keys & Mouse Implementation **************/

	/**
	* @var std::unordered_map<int, KeyProperties> kb_mouse_input_properties_map
	* @brief Stores the input an its properties. (ONLY FOR KEYBOARD AND MOUSE)
	*/
	std::unordered_map<int, KeyProperties> kb_mouse_input_properties_map_;

	/**
	* @var std::map<int, std::set<int>> kb_mouse_action_map
	* @brief Stores the actions an it's binds keys. (ONLY FOR KEYBOARD AND MOUSE)
	*/
	std::map<int, std::set<int>> kb_mouse_action_map_;

	/**
	* @brief Assign the properties of the recive key, checking the action value.
	*
	* @param key The key to recive the action
	* @param input_type Mainly use the diferentiate the type of input(-1 == mouse), if is keyboard then is the system-specific scancode of the key
	* @param action Recive the action(GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT)
	* @param mods Bit field describing which modifier keys were held down.
	*/
	void kb_mouse_callback(int key, int input_type, int action, int mods);

	/**
	* @brief Update the value of the cursor position
	*
	* @param xpos Cursor's X position
	* @param ypos Cursor's Y position
	*/
	void cursor_position_callback(double xpos, double ypos);

	/**
	* @brief Update the value of the scroll
	*
	* @param xpos Scroll's X off set
	* @param ypos Scroll's Y off set
	*/
	void scrooll_callback(double xoffset, double yoffset);


	/****************** Gamepad Implementation **************/

	/**
	* @var std::map<int, GamepadState> game_pad_states
	* @brief Stores the input an its properties. (ONLY FOR GAMEPAD)
	*/
	std::map<int, GamepadState> game_pad_states_;

	/**
	* @var std::map<int, std::vector<GamePadAction>> gamepad_action_map
	* @brief Stores the actions an it's binds buttons. (ONLY FOR GAMEPAD)
	*/
	std::map<int, std::vector<GamePadAction>> gamepad_action_map_;

	/**
	* @var std::map<int, std::vector<GamePadAction>> gamepad_action_map
	* @brief Stores the actions an it's binds axis. (ONLY FOR GAMEPAD)
	*/
	std::map<int, GamePadAction> gamepad_axis_action_map_;


	/****************** Static Properties and Functions **************/

	/**
	* @var static std::unordered_map<GLFWwindow*, LavaInput*> input_map
	* @brief Stores the Window and the input associated to it.
	*/
	static std::unordered_map<GLFWwindow*, LavaInput*> s_input_map_;

	/**
	* @brief Call the specific input associated to the recive window
	*		 assigning the properties of the recive key, checking the action value. (FOR KEYBOARD)
	*
	* @param window The window to call
	* @param key The key to recive the action
	* @param scancode The system-specific scancode of the key
	* @param action Recive the action(GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT)
	* @param mods Bit field describing which modifier keys were held down.
	*/
	static void global_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	/**
	* @brief Call the specific input associated to the recive window
	*		 assigning the properties of the recive key, checking the action value. (FOR MOUSE)
	*
	* @param window The window to call
	* @param button The button to recive the action
	* @param action Recive the action(GLFW_PRESS or GLFW_RELEASE)
	* @param mods Bit field describing which modifier keys were held down.
	*/
	static void global_mouse_callback(GLFWwindow* window, int button, int action, int mods);


	/**
	* @brief Call the specific function associated to the recive window
	*		 assigning cursor's postition value, checking the action value. (FOR MOUSE)
	*
	* @param window The window to call
	* @param xpos Cursor's X position
	* @param ypos Cursor's X position
	*/
	static void global_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

	/**
	* @brief Update the value of the scroll
	*
	* @param window The window to call
	* @param xpos Scroll's X off set
	* @param ypos Scroll's Y off set
	*/
	static void global_scrooll_callback(GLFWwindow* window, double xoffset, double yoffset);

	/**
	* @brief Clear the specific input associated to the recive window
	*
	* @param window The window to clear
	*/
	static void clean_bindings(GLFWwindow* window);

	/**
	* @brief Process the end frame, for the input, in all the windows in input_map
	*
	* @param window The window to clear
	*/
	static void end_frame();
};

#endif