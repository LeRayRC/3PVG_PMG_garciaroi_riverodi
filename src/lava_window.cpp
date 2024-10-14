/**
 * @file lava_window.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Window's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "lava_window.hpp"

LavaWindow::LavaWindow(unsigned int x, unsigned int y, const char* name) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  w_ = glfwCreateWindow(x, y, name, nullptr, nullptr);
}


GLFWwindow* LavaWindow::get_window() const{
  return w_;
}
