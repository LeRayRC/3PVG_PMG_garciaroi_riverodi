#include "lava/common/lava_global_helpers.hpp"

void GenericUpdateWithInput(size_t id, LavaECSManager* ecs_manager, LavaEngine& engine) {

  //Get input from the engine current window
  LavaInput* input = engine.window_.get_input();
  glm::vec3 input_vector = glm::vec3(0.0f);

  if (input->isInputDown(KEY_D) || input->isInputDown(KEY_RIGHT)) {
    input_vector.x = 1.0f;
  }
  else if (input->isInputDown(KEY_A) || input->isInputDown(KEY_LEFT)) {
    input_vector.x = -1.0f;
  }

  if (input->isInputDown(KEY_W) || input->isInputDown(KEY_UP)) {
    input_vector.y = 1.0f;
  }
  else if (input->isInputDown(KEY_S) || input->isInputDown(KEY_DOWN)) {
    input_vector.y = -1.0f;
  }

  const float epsilon = 1e-6f; // 0.000001
  float magnitude = glm::length(input_vector);
  if (magnitude > epsilon) {
    input_vector = (glm::normalize(input_vector) * (float)engine.dt_);
    auto transform_optional = ecs_manager->getComponent<TransformComponent>(id);
    if (transform_optional) {
      auto& transform_comp = transform_optional->value();
      transform_comp.pos_ += input_vector;
    }
  }
}