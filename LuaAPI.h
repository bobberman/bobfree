#pragma once

#include "utils/c_singleton.h"
#include "utils/defs.h"
#include "macros.h"

/*
Hook list
-=-=-=-=-

Draw-call-safe: (basically you can run calls to draw functions here and not have to worry about crashes, generally)
	MenuBegin		- runs before the menu is rendered
	MenuEnd			- runs after the whole menu has been rendered.
	FrameRender		- runs before the menu & everything else is rendered in Present hook

*/

// forward defs
struct lua_State;
class c_user_cmd;

// definitions to make coding the api somewhat easier

// create a symbol for the lua api with a specified name
#define LUA_FUNC_NAME(name)					LAPI_##name

// create a c(++) function that can be called by lua
#define LUA_C_FUNC(name)					int LUA_FUNC_NAME(name)  (lua_State* L)

// create a library function that can be called from lua (libname.func)
#define LUA_C_LIBFUNC(lib, func)			LUA_C_FUNC( lib ## _ ## func)

// begin definition of a new module table that contains functions
#define LUA_BEGINTABLEDEF(name)				luaL_Reg module_ ## name ## [] = {

// end definition of a module table
#define LUA_ENDTABLEDEF()					{ NULL, NULL } };

// this is to make adding functions to the function tables much easier
#define LUA_TABLE_FUNC(lib, name)			{ #name, LUA_FUNC_NAME(lib ## _ ## name) },

// load a table of functions that are properly named into a lua lib
#define LUA_LOAD_MODULE_TABLE(tabl)			luaL_newlib(L, module_ ## tabl); lua_setglobal(L, _(#tabl));

// makes checking arguments easier and it always causes the same error, which is helpful
#define LUA_ARG_CHECK_(nargs, retcount)		if (lua_gettop(L) != nargs) { luaL_error(L, _("Function expected arg count of %d"), nargs); return retcount; }

// shorten out the returncount thing
#define LUA_ARG_CHECK(nargs)				LUA_ARG_CHECK_(nargs, 0)

// define a global integer value
#define LUA_DEF_G_INT_(name, var)			lua_pushinteger(L, var);lua_setglobal(L, _(name));

// shorten out the name and use the var sym name as the name
#define LUA_DEF_G_INT(var)					LUA_DEF_G_INT_(var, #var)

// make a function much more pretty
#define LUA_ARG_CLR							LUA_ARG_CLR_ // uff ya b4d code

// CreateMove add boolean to the table param
#define LUA_CM_BOOLEAN(name)				lua_pushboolean(L, cmd-> ## name); lua_setfield(L, -2, _(#name));

// CreateMove add integer to the table param
#define LUA_CM_INTEGER(name)				lua_pushinteger(L, cmd-> ## name); lua_setfield(L, -2, _(#name));

// CreateMove add float to the table param
#define LUA_CM_FLOAT(name)					lua_pushnumber(L, cmd-> ## name); lua_setfield(L, -2, _(#name));

// CreateMove add vector to the table param, each item in the vec gets converted to "<item name>_<xyz>" as individual floats (technically doubles)
#define LUA_CM_VECTOR(name)					lua_pushnumber(L, cmd-> ## name ## .x); lua_setfield(L, -2, _(#name "_x"));lua_pushnumber(L, cmd-> ## name ## .y); lua_setfield(L, -2, _(#name "_y"));lua_pushnumber(L, cmd-> ## name ## .z); lua_setfield(L, -2, _(#name "_z"));

// shitty ass conversions
#define IMVEC4_TO_C_COLOR(clr)				c_color(clr.Value.x * 255, clr.Value.y * 255, clr.Value.z * 255)

// // // // // // // // // // // // // // //
// these functions cannot be a part of the singleton class because that's not how it works (__stdcall bullshit)
LUA_C_LIBFUNC(config,	ChangeFOV);
LUA_C_LIBFUNC(config,	ChangeBhop);

LUA_C_LIBFUNC(hook,		Add);
LUA_C_LIBFUNC(hook,		Remove);

LUA_C_LIBFUNC(draw,		Text);
LUA_C_LIBFUNC(draw,		Line);
LUA_C_LIBFUNC(draw,		Rect);
LUA_C_LIBFUNC(draw,		RectFilled);
LUA_C_LIBFUNC(draw,		RectLinearGrad);

LUA_C_LIBFUNC(menu,		Start);
LUA_C_LIBFUNC(menu,		End);
LUA_C_LIBFUNC(menu,		Text);
LUA_C_LIBFUNC(menu,		TextColored);

LUA_C_LIBFUNC(game, IsInGame);
LUA_C_LIBFUNC(game, GetPlayers);
LUA_C_LIBFUNC(game, LocalPlayer);

LUA_C_LIBFUNC(player, IsAlive);
LUA_C_LIBFUNC(player, IsDormant);
LUA_C_LIBFUNC(player, GetName);
LUA_C_LIBFUNC(player, GetScreenPos);

LUA_C_FUNC(Color);
// // // // // // // // // // // // // // //

class LuaSystem : public c_singleton<LuaSystem> {
public:

	void Setup();

	void RunString(std::string str);

	bool m_bShowLuaEditor = false;

	void ExecuteHook(std::string type);

	void ExecuteCreateMove(c_user_cmd* cmd);

	void ClearHooks();

private:

	lua_State* L;
};

#define LUASYS LuaSystem::instance()