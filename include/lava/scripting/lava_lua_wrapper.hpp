#ifndef __LAVA_LUA_WRAPPER__ 
#define __LAVA_LUA_WRAPPER__ 1

#include "lava/common/lava_types.hpp"

#include "lua.hpp"

int ECS_CreateEntity(lua_State* L);
int ECS_AddComponent_Transform(lua_State* L);
int ECS_AddComponent_Render(lua_State* L);

//glm::vec3 luaTableToVec3(lua_State* L, int index);
//void vectorToLuaTable(lua_State* L, glm::vec3);

#endif // !__LAVA_LUA_WRAPPER__ 
