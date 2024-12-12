#include "ecs/lava_rotate_system.hpp"

LavaRotateSystem::LavaRotateSystem()
{
}

LavaRotateSystem::~LavaRotateSystem()
{
}

void LavaRotateSystem::operator()(std::vector<std::optional<RotateComponent>>& rotate_component_vector,
  std::vector<std::optional<TransformComponent>>& transform_component_vector, const float dt) const {
  auto rot_it = rotate_component_vector.begin();
  auto rot_end = rotate_component_vector.end();
  auto tr_it = transform_component_vector.begin();
  auto tr_end = transform_component_vector.end();
  for (; rot_it != rot_end && tr_it != tr_end; rot_it++, tr_it++) {
    if (!rot_it->has_value()) continue;
    if (!tr_it->has_value()) continue;
    auto& rotate_component = rot_it->value();
    auto& transform_component = tr_it->value();

    //Do things
    if (rotate_component.can_rotate_) {
      transform_component.rot_.x += (rotate_component.rotate_speed_.x * dt);
      transform_component.rot_.y += (rotate_component.rotate_speed_.y * dt);
      transform_component.rot_.z += (rotate_component.rotate_speed_.z * dt);
    }
  }
}

