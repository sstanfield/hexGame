#include "hexlua.h"
#include "Application.h"
#include "tb_widgets_reader.h"

#include <iostream>

namespace hexgame {
namespace lua {


static void tb_addWidget(const char *parent, const char *source) {
	tb::TBWidget *p = Application::instance()->getRootWidget()->GetWidgetByID(parent);
	if (p) {
		tb::g_widgets_reader->LoadData(p, source);
	} else printf("ERROR: Unable to find parent widget: %s\n", parent);
}

static int tb_getHeight(const char *widget) {
	tb::TBWidget *w = Application::instance()->getRootWidget()->GetWidgetByID(widget);
	int ret = -1;
	if (w) {
		tb::TBRect r = w->GetRect();
		ret = r.h;
	} else printf("ERROR: Unable to find widget: %s\n", widget);
	return ret;
}

static int tb_getWidth(const char *widget) {
	tb::TBWidget *w = Application::instance()->getRootWidget()->GetWidgetByID(widget);
	int ret = -1;
	if (w) {
		tb::TBRect r = w->GetRect();
		ret = r.w;
	} else printf("ERROR: Unable to find widget: %s\n", widget);
	return ret;
}


HexLua::HexLua() {
	state = luaL_newstate();
	if (state) {
		luaL_openlibs(state);
		BIND_FUNCTION_TO_LUA(state, tb_addWidget);
		BIND_FUNCTION_TO_LUA(state, tb_getHeight);
		BIND_FUNCTION_TO_LUA(state, tb_getWidth);
	} else throw std::runtime_error("Failed to init LUA.");
}

HexLua::~HexLua() {
	lua_close(state);
}

void HexLua::addStringVar(const std::string& name, const std::string& val) {
	lua_pushlstring(state, val.c_str(), val.size());
	lua_setglobal(state, name.c_str());
}

std::string HexLua::getStringVar(const std::string& name) {
	lua_getglobal(state, name.c_str());
	const char *v = lua_tostring(state, -1);
	if (v) return std::string(v);
	return "";
}

void HexLua::addTable(const std::string& name, const std::unordered_map<std::string, std::string>& map) {
	lua_newtable(state);

	for (auto& i : map) {
		lua_pushlstring(state, i.first.c_str(), i.first.size());
		lua_pushlstring(state, i.second.c_str(), i.second.size());
		lua_rawset(state, -3);
	}
	lua_setglobal(state, name.c_str());
}

void HexLua::updateTable(const std::string& name, std::unordered_map<std::string, std::string>& map) {
	lua_getglobal(state, name.c_str());
	if (!lua_istable(state, -1)) {
		std::cout << name << " is not a valid table.\n";
		return;
	}
	for (auto& i : map) {
		lua_pushstring(state, i.first.c_str());
		lua_gettable(state, -2);
		const char* v = lua_tostring(state, -1);
		i.second = v;
		lua_pop(state, 1);
	}
}

void HexLua::runScript(const std::string& script) {
	if (luaL_dofile(state, script.c_str())) {
		printf("ERROR: %s\n", lua_tostring(state, -1));
	}
}

}
}
