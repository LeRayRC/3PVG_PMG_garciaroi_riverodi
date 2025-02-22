#include "lava/ecs/lava_update_system.hpp"
#include "lava/engine/lava_engine.hpp"

LavaUpdateSystem::LavaUpdateSystem(class LavaEngine& engine) : engine_{engine} {

}
LavaUpdateSystem::~LavaUpdateSystem() {

}

void LavaUpdateSystem::update(std::vector<std::optional<UpdateComponent>>& update_vector) {
  auto update_it = update_vector.begin();
  auto update_end = update_vector.end();

  size_t id = 0;

  for (; update_it != update_end; update_it++ , id++) {
    if (!update_it->has_value()) continue;

    auto& update_comp = update_it->value();
    update_comp.update_(update_comp.id, update_comp.ecs_manager, engine_);
  }
}
