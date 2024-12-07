#include "scripting/lava_lua_script.hpp"

LavaLuaScript::LavaLuaScript()
	: state_{ luaL_newstate(),&lua_close }
	, s{ state_.get() } {
	luaL_openlibs(s);
}

LavaLuaScript::~LavaLuaScript()
{
}

void LavaLuaScript::run(const std::string& str) {
	if (luaL_dofile(s, str.c_str()) != LUA_OK) {
		std::cerr << "Error executing Lua script: " << lua_tostring(s, -1) << std::endl;
		throw std::runtime_error("Lua error loading script");
	}
}


void LavaLuaScript::check(int error) {
	if (error != LUA_OK) {
		std::string err = lua_tostring(s, lua_gettop(s));
		lua_pop(s, lua_gettop(s));
		throw std::runtime_error("Lua error: " + err);
	}
}