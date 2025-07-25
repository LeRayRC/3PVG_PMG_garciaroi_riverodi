/**
 * @file lava_global_helpers.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Global Helpers header's file
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_GLOBAL_HELPERS_H__
#define __LAVA_GLOBAL_HELPERS_H__ 1

#include "lava/common/lava_types.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/input/lava_input.hpp"
#include "lava/engine/lava_engine.hpp"
#include "lava/ecs/lava_ecs_components.hpp"
#include "lava/openxr_common/HelperFunctions.h"


enum GraphicsAPI_Type : uint8_t {
  UNKNOWN,
  D3D11,
  D3D12,
  OPENGL,
  OPENGL_ES,
  VULKAN
};

const char* GetGraphicsAPIInstanceExtensionString(GraphicsAPI_Type type);

static inline glm::mat4 GenerateViewMatrix(const glm::vec3& pos, const glm::vec3& rot) {
  float pitch = glm::radians(rot.x); // Rotaci�n en el eje X
  float yaw = glm::radians(rot.y);   // Rotaci�n en el eje Y
  float roll = glm::radians(rot.z);

  glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3{ 1.f, 0.f, 0.f });
  glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3{ 0.f, -1.f, 0.f });

  glm::mat4 rotationMatrix = glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);

  glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), pos);
  return glm::inverse(cameraTranslation * rotationMatrix);


  //return glm::lookAt(pos, pos + forward, glm::vec3(0.0f, -1.0f,0.0f));
}

static inline glm::vec3 CalculateForwardVector(const glm::vec3& rotation_degrees) {
  
  float yaw = glm::radians(rotation_degrees.y);   
  float pitch = glm::radians(rotation_degrees.x); 

  //glm::vec3 forward;
  //forward.x = cosf(yaw) * cosf(pitch);
  //forward.y = sinf(pitch);
  //forward.z = sinf(yaw) * cosf(pitch);

  //forward = glm::normalize(forward);

  // Crear la matriz de rotación (Creo que esta de reves, por lo menos con respecto a GenerateViewMatrix)
  glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0, -1, 0)); // Rotación en Y
  rotationMatrix = glm::rotate(rotationMatrix, pitch, glm::vec3(1, 0, 0));         // Rotación en X

  // Forward vector inicial (eje Z negativo)
  glm::vec4 forward = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

  // Aplicar la rotación
  forward = rotationMatrix * forward;

  // Devolver el forward vector normalizado
  return glm::normalize(glm::vec3(forward));

  //return forward;
}


static inline void GenericUpdateWithInput(size_t id, LavaECSManager* ecs_manager, LavaEngine& engine) {
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
      transform_comp.pos_ += (input_vector);
    }
  }
}

/**
 * @brief Generic Flycam implementation function for a camera entity.
 * 
 * @param id entity id of the camera
 * @param ecs_manager reference to the ecs manager to retrieve other components
 * @param engine reference to the engine to get delta time and other things
 */
static inline void UpdateCameraWithInput(size_t id, LavaECSManager* ecs_manager, LavaEngine& engine) {
  //Get input from the engine current window
  LavaInput* input = engine.window_.get_input();

  float camera_speed = 2.0f;
  float alpha = 0.0f;
  float omega = 0.0f;
  glm::vec2 mouse_position;
  glm::vec3 forward_vector;

  if (input->isInputDown(MOUSE_BUTTON_2)) {
    mouse_position = input->getMousePosition();


    omega = (mouse_position.y / engine.window_extent_.height) * 3.14f;
    alpha = (mouse_position.x / engine.window_extent_.width) * 6.28f;

    if (alpha > 6.28) alpha = 6.28f;
    if (alpha < 0.0f) alpha = 0.0f;

    auto transform_optional = ecs_manager->getComponent<TransformComponent>(id);
    if (transform_optional) {
      auto& transform_comp = transform_optional->value();
      transform_comp.rot_ = glm::vec3(
        -1.0f * glm::degrees(omega - 1.57f),
        glm::degrees(alpha - 3.14f),
        0.0f
      );

      forward_vector = CalculateForwardVector(transform_comp.rot_);
      

      if (input->isInputDown(KEY_W)) {
        transform_comp.pos_ += ((float)engine.dt_ * camera_speed * forward_vector);
      }
      else if (input->isInputDown(KEY_S)) {
        transform_comp.pos_ -= ((float)engine.dt_ * camera_speed *  forward_vector);
      }
      if (input->isInputDown(KEY_D)) {
        glm::vec3 right = glm::cross(forward_vector, glm::vec3(0.0f, 1.0f, 0.0f));
        transform_comp.pos_ += ((float)engine.dt_ * camera_speed * glm::normalize(right));
      }
      else if (input->isInputDown(KEY_A)) {
        glm::vec3 right = glm::cross(forward_vector, glm::vec3(0.0f, 1.0f, 0.0f));
        transform_comp.pos_ -= ((float)engine.dt_ * camera_speed * glm::normalize(right));
      }
    }
  }
}

void allocate_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector);
void update_lights(std::vector<std::optional<struct LightComponent>>& light_component_vector,
  std::vector<std::optional<struct TransformComponent>>& transform_vector);

#endif // !__LAVA_GLOBAL_HELPERS_H__
