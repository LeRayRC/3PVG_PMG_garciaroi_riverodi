#include "lava/ecs/lava_lua_system.hpp"

LavaLuaSystem::LavaLuaSystem()
{
}

LavaLuaSystem::~LavaLuaSystem()
{
}

void LavaLuaSystem::run(std::vector<std::optional<LuaScriptComponent>>& lua_component_vector) {
  auto lua_it = lua_component_vector.begin();
  auto lua_end = lua_component_vector.end();
  for (; lua_it != lua_end; lua_it++) {
    if (!lua_it->has_value()) continue;
    auto& lua_script_component = lua_it->value();

    //lua_script_.run(lua_it->value().lua_script_path_);
  }
}

