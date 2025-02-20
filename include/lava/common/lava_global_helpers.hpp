#ifndef __LAVA_GLOBAL_HELPERS_H__
#define __LAVA_GLOBAL_HELPERS_H__ 1

#include "lava/common/lava_types.hpp"

static inline glm::mat4 GenerateViewMatrix(glm::vec3& pos, glm::vec3& rot) {
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

  glm::vec3 forward;
  forward.x = cosf(yaw) * cosf(pitch);
  forward.y = sinf(pitch);
  forward.z = sinf(yaw) * cosf(pitch);

  forward = glm::normalize(forward);

  return forward;
}
#endif // !__LAVA_GLOBAL_HELPERS_H__
