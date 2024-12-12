#ifndef __LAVA_LUA_SYSTEM_H_
#define __LAVA_LUA_SYSTEM_H_ 1

#include "lava_types.hpp"
#include "scripting/lava_lua_script.hpp"
#include "ecs/lava_ecs_components.hpp"

class LavaLuaSystem
{
public:
	LavaLuaSystem();
	~LavaLuaSystem();

	void run(std::vector<std::optional<LuaScriptComponent>>&);

private:
	LavaLuaScript lua_script_;
};





#endif // !__LAVA_LUA_SYSTEM_H

