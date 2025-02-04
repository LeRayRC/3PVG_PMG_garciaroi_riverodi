#ifndef __LAVA_GLOBAL_HELPERS_H__
#define __LAVA_GLOBAL_HELPERS_H__ 1

#include "lava_types.hpp"

static inline glm::mat4 GenerateViewMatrix(glm::vec3& pos, glm::vec3& rot) {
  float pitch = glm::radians(rot.x); // Rotación en el eje X
  float yaw = glm::radians(rot.y);   // Rotación en el eje Y
  float roll = glm::radians(rot.z);

  glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3{ 1.f, 0.f, 0.f });
  glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3{ 0.f, -1.f, 0.f });

  glm::mat4 rotationMatrix = glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);

  glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), pos);
  return glm::inverse(cameraTranslation * rotationMatrix);
}
#endif // !__LAVA_GLOBAL_HELPERS_H__
