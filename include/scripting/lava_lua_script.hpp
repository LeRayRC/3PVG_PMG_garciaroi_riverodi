#include "lua.hpp"
#include <memory>
#include <stdexcept>
#include <iostream>


#ifndef __LAVA_LUA_SCRIPT__
#define __LAVA_LUA_SCRIPT__ 1

class LavaLuaScript
{
public:
	LavaLuaScript();
	~LavaLuaScript();

	void check(int error);

	void run(const std::string& str);

	void add_global(int(*function)(lua_State*), const char* name) {
		lua_pushcfunction(s, function);
		lua_setglobal(s, name);
	}

private:
	std::unique_ptr<lua_State, decltype(&lua_close)> state_;
	lua_State* s;
};

#endif // !__LAVA_LUA_SCRIPT__
