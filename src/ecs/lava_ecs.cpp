#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_ecs_components.hpp"

LavaECSManager::LavaECSManager(){
  last_entity = 0;
  addComponentType<RenderComponent>();
  addComponentType<TransformComponent>();
  addComponentType<CameraComponent>();
  addComponentType<LuaScriptComponent>();
  addComponentType<RotateComponent>();
  addComponentType<LightComponent>();
  addComponentType<UpdateComponent>();
}

LavaECSManager::~LavaECSManager()
{
}

size_t LavaECSManager::createEntity() {
  size_t entity_id;
  if (!free_slots.empty()) {
    entity_id = free_slots.back();
    free_slots.pop_back();
  }
  else {
    entity_id = last_entity++;
    for (auto& [key, component_list] : component_list_map_) {
      component_list->resize(last_entity);
    }
  }
  return entity_id;
}

void LavaECSManager::deleteEntity(size_t entity_id){
  for (auto& [key, component_list] : component_list_map_) {
    if (entity_id < component_list->getSize()) {
      component_list->remove(entity_id);
    }
  }
  free_slots.push_back(entity_id);
}
