#include "scripting/lava_lua_wrapper.hpp"
#include "ecs/lava_ecs.hpp"
#include "lava_world.hpp"


int ECS_CreateEntity(lua_State* L) {
  LavaECSManager* ecs_manager = LavaWorld::GetWorld().getECSManager();
  size_t entity = ecs_manager->createEntity();
  lua_pushinteger(L, entity);
  return 1;
}

int ECS_AddComponent_Transform(lua_State* L) {
  size_t entity = luaL_checkinteger(L, 1);

  LavaECSManager* ecs_manager = LavaWorld::GetWorld().getECSManager();
  if (ecs_manager->addComponent<TransformComponent>(entity) != ECS_SUCCESS) {
    lua_pushinteger(L, -1);
  }
  else {
    lua_pushinteger(L, ECS_SUCCESS);
  }
  return 1;
}

int ECS_AddComponent_Render(lua_State* L) {
  size_t entity = luaL_checkinteger(L, 1);

  LavaECSManager* ecs_manager = LavaWorld::GetWorld().getECSManager();
  if (ecs_manager->addComponent<RenderComponent>(entity) != ECS_SUCCESS) {
    lua_pushinteger(L, -1);
  }
  else {
    lua_pushinteger(L, ECS_SUCCESS);
  }
  return 1;
}


//glm::vec3 luaTableToVec3(lua_State* L, int index){
//  luaL_checktype(L, index, LUA_TTABLE); // Verifica que sea una tabla
//
//  glm::vec3 vec;
//
//  // Get x component
//  lua_getfield(L, index, "x");
//  if (!lua_isnumber(L, -1)) {
//    lua_pop(L, 1); // Limpia la pila
//    throw std::runtime_error("Expected 'x' to be a number");
//  }
//  vec.x = static_cast<float>(lua_tonumber(L, -1));
//  lua_pop(L, 1); // Limpia la pila
//
//  // Get y component
//  lua_getfield(L, index, "y");
//  if (!lua_isnumber(L, -1)) {
//    lua_pop(L, 1);
//    throw std::runtime_error("Expected 'y' to be a number");
//  }
//  vec.y = static_cast<float>(lua_tonumber(L, -1));
//  lua_pop(L, 1);
//
//  // Get z component
//  lua_getfield(L, index, "z");
//  if (!lua_isnumber(L, -1)) {
//    lua_pop(L, 1);
//    throw std::runtime_error("Expected 'z' to be a number");
//  }
//  vec.z = static_cast<float>(lua_tonumber(L, -1));
//  lua_pop(L, 1);
//
//}

//void vectorToLuaTable(lua_State* L, const std::vector<int>& vec) {
//  lua_newtable(L);  // Crear una nueva tabla en la pila
//  for (size_t i = 0; i < vec.size(); ++i) {
//    lua_pushinteger(L, i + 1);  // Lua usa índices basados en 1
//    lua_pushinteger(L, vec[i]);
//    lua_settable(L, -3);  // Tabla en -3
//  }
//}