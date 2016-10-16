#pragma once

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include <string>
#include <utility>
#include <unordered_map>
#include <cassert>

namespace hexgame {
namespace lua {

// Code originally from http://inspired-code.blogspot.com.au/p/automagical-language-bindings-using-c.html

// If we are wrapping/binding a function that returns a char *, propagate the result to lua
template <typename... As, typename... Ps>
int MakeCallableFunc(lua_State* L, int, char * (*fn)(Ps... values), char * (*)(), As... params) {
	char *r = fn(std::forward<As>(params)...);
	lua_pushstring(L, r);
	return 1;
}

// If we are wrapping/binding a function that returns a string, propagate the result to lua
template <typename... As, typename... Ps>
int MakeCallableFunc(lua_State* L, int, std::string (*fn)(Ps... values), std::string (*)(), As... params) {
	std::string r = fn(std::forward<As>(params)...);
	lua_pushstring(L, r.c_str());
	return 1;
}

// If we are wrapping/binding a function that returns an double, propagate the result to lua
template <typename... As, typename... Ps>
int MakeCallableFunc(lua_State* L, int, double (*fn)(Ps... values), double (*)(), As... params) {
	double r = fn(std::forward<As>(params)...);
	lua_pushnumber(L, r);
	return 1;
}

// If we are wrapping/binding a function that returns an float, propagate the result to lua
template <typename... As, typename... Ps>
int MakeCallableFunc(lua_State* L, int, float (*fn)(Ps... values), float (*)(), As... params) {
	float r = fn(std::forward<As>(params)...);
	lua_pushnumber(L, r);
	return 1;
}

// If we are wrapping/binding a function that returns an int, propagate the result to lua
template <typename... As, typename... Ps>
int MakeCallableFunc(lua_State* L, int, int (*fn)(Ps... values), int (*)(), As... params) {
	int r = fn(std::forward<As>(params)...);
	lua_pushinteger(L, r);
	return 1;
}

// Otherwise if the return type is not int, return 0
template <typename R, typename... As, typename... Ps>
int MakeCallableFunc(lua_State*, int, R (*fn)(Ps... values), R (*)(), As... params) {
	fn(std::forward<As>(params)...);
	return 0;
}

// Handle int arguments
template <typename R, typename... As, typename... Ts, typename... Ps>
int MakeCallableFunc(lua_State* L, int i, R (*fn)(As... values), R (*)(int, Ts... values), Ps... params) {
	R (*fn3)(Ts... values);
	fn3 = nullptr;
	assert(lua_isinteger(L,i));
	return MakeCallableFunc(L, i+1, fn, fn3, std::forward<Ps>(params)..., (int)lua_tointeger(L, i));
}

// Handle double arguments
template <typename R, typename... As, typename... Ts, typename... Ps>
int MakeCallableFunc(lua_State* L, int i, R (*fn)(As... values), R (*)(double, Ts... values), Ps... params) {
	R (*fn3)(Ts... values);
	fn3 = nullptr;
	assert(lua_isnumber(L,i));
	return MakeCallableFunc(L, i+1, fn, fn3, std::forward<Ps>(params)..., (double)lua_tonumber(L, i));
}

// Handle string arguments
template <typename R, typename... As, typename... Ts, typename... Ps>
int MakeCallableFunc(lua_State* L, int i, R (*fn)(As... values), R (*)(const char*, Ts... values), Ps... params) {
	R (*fn3)(Ts... values);
	fn3 = nullptr;
	assert(lua_isstring(L,i));
	return MakeCallableFunc(L, i+1, fn, fn3, std::forward<Ps>(params)..., (const char*)lua_tostring(L, i));
}

// Add helper for entry in to creating the binding
template <typename R, typename... As>
int MakeCallableFunc(lua_State* L, R (*fn)(As... values)) {
	return MakeCallableFunc(L, 1, fn, fn);
}

#define BIND_FUNCTION_TO_LUA(luaState, func) \
	lua_pushcclosure(luaState, [](lua_State*lua)->int { return MakeCallableFunc(lua, func); }, 0); \
	lua_setglobal(luaState, #func)

class HexLua {
public:
	HexLua();
	virtual ~HexLua();

/*	template <typename R, typename... As>
	int BindFuncToLua(const std::string name, R (*func)(As... values)) {
		BIND_FUNCTION_TO_LUA(state, func);
		auto lcloser = [func](lua_State* lua)->int {
			return hexgame::lua::MakeCallableFunc(lua, func);
		};
		lua_pushcclosure(state, lcloser, 0);
		//lua_pushcclosure(state, [](lua_State* lua)->int {
		//		return 0;//hexgame::lua::MakeCallableFunc(lua, func);
		//	}, 0);
		lua_setglobal(state, name.c_str());
	}*/

	void addStringVar(const std::string& name, const std::string& val);
	std::string getStringVar(const std::string& name);
	void addTable(const std::string& name, const std::unordered_map<std::string, std::string>& map);
	void updateTable(const std::string& name, std::unordered_map<std::string, std::string>& map);
	void runScript(const std::string& script);
	
private:
	lua_State *state = nullptr;
};

}
}
