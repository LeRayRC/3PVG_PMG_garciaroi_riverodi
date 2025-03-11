/**
 * @page how_to_input Input
 * 
 * 
 * @section section_render_input_window Input Window
 * 
 * The input system is linked to the engine window. You can get the input system this way
 *
 * @code {.hpp}
 * #include "lava/engine/lava_engine.hpp"
 * #include "lava/input/lava_input.hpp"
 * 
 * @endcode
 * 
 * 
 * @code {.cpp}
 *  LavaEngine engine(1920,1080);
 *  LavaInput* input = engine.window_.get_input();
 * @endcode
 * 
 * 
 * @section section_input_mapping Input Bindings 
 * 
 * All the mapping keys can be found in the file lava_input.hpp. 
 * If you need more KEY BINDINGS you can refer to the glfw @link https://www.glfw.org/docs/3.0/group__input.html documentation @endlink 
 * 
 * 
 * @section section_input_detection Input Detection
 * 
 * Here is an exmple of the input detection from the GenericUpdateWithInput function located at lava_global_helpers.hpp file
 * 
 * @code {.cpp}
 * glm::vec3 input_vector = glm::vec3(0.0f);
 * 
 * if (input->isInputDown(KEY_D) || input->isInputDown(KEY_RIGHT)) {
 *   input_vector.x = 1.0f;
 * }
 * else if (input->isInputDown(KEY_A) || input->isInputDown(KEY_LEFT)) {
 *   input_vector.x = -1.0f;
 * }
 * 
 * if (input->isInputDown(KEY_W) || input->isInputDown(KEY_UP)) {
 *   input_vector.y = 1.0f;
 * }
 * else if (input->isInputDown(KEY_S) || input->isInputDown(KEY_DOWN)) {
 *   input_vector.y = -1.0f;
 * }
 * @endcode
 * 
 * 
 * 
 */