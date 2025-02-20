#ifndef __LAVA_LUA_SYSTEM_H_
#define __LAVA_LUA_SYSTEM_H_ 1

#include "lava/common/lava_types.hpp"
#include "lava/scripting/lava_lua_script.hpp"
#include "lava/ecs/lava_ecs_components.hpp"

struct LuaScriptComponent;

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

