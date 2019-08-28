#include "Lua/lua.hpp"
#include "utils/c_config.h"
#include "LuaAPI.h"
#include "ImConsole.h"
#include "hooks/idirect3ddevice9.h"
#include "sdk/c_user_cmd.h"
#include "sdk/c_client_entity_list.h"

LUA_BEGINTABLEDEF(config)
	LUA_TABLE_FUNC(config, ChangeFOV)
	LUA_TABLE_FUNC(config, ChangeBhop)
LUA_ENDTABLEDEF()

LUA_BEGINTABLEDEF(hook)
	LUA_TABLE_FUNC(hook, Add)
	LUA_TABLE_FUNC(hook, Remove)
LUA_ENDTABLEDEF()

LUA_BEGINTABLEDEF(draw)
	LUA_TABLE_FUNC(draw, Text)
	LUA_TABLE_FUNC(draw, Line)
	LUA_TABLE_FUNC(draw, Rect)
	LUA_TABLE_FUNC(draw, RectFilled)
LUA_ENDTABLEDEF()

LUA_BEGINTABLEDEF(menu)
	LUA_TABLE_FUNC(menu, Start)
	LUA_TABLE_FUNC(menu, End)
	LUA_TABLE_FUNC(menu, Text)
	LUA_TABLE_FUNC(menu, TextColored)
LUA_ENDTABLEDEF()

void LuaSystem::Setup() {
	L = luaL_newstate();
	luaL_openlibs(L);

	LUA_LOAD_MODULE_TABLE(config);
	LUA_LOAD_MODULE_TABLE(hook);
	LUA_LOAD_MODULE_TABLE(draw);
	LUA_LOAD_MODULE_TABLE(menu);

	// color "macro" to make color usage generally easier
	lua_pushcfunction(L, LUA_FUNC_NAME(Color));
	lua_setglobal(L, _("Color"));

	LUA_DEF_G_INT_("IN_ATTACK",		c_user_cmd::flags::attack);
	LUA_DEF_G_INT_("IN_JUMP",		c_user_cmd::flags::jump);
	LUA_DEF_G_INT_("IN_DUCK",		c_user_cmd::flags::duck);
	LUA_DEF_G_INT_("IN_FORWARD",	c_user_cmd::flags::forward);
	LUA_DEF_G_INT_("IN_BACK",		c_user_cmd::flags::back);
	LUA_DEF_G_INT_("IN_USE",		c_user_cmd::flags::use);
	LUA_DEF_G_INT_("IN_CANCEL",		c_user_cmd::flags::cancel);
	LUA_DEF_G_INT_("IN_LEFT",		c_user_cmd::flags::left);
	LUA_DEF_G_INT_("IN_RIGHT",		c_user_cmd::flags::right);
	LUA_DEF_G_INT_("IN_MOVELEFT",	c_user_cmd::flags::move_left);
	LUA_DEF_G_INT_("IN_MOVERIGHT",	c_user_cmd::flags::move_right);
	LUA_DEF_G_INT_("IN_ATTACK2",	c_user_cmd::flags::attack2);
	LUA_DEF_G_INT_("IN_RUN",		c_user_cmd::flags::run);
	LUA_DEF_G_INT_("IN_RELOAD",		c_user_cmd::flags::reload);
	LUA_DEF_G_INT_("IN_ALT1",		c_user_cmd::flags::alt1);
	LUA_DEF_G_INT_("IN_ALT2",		c_user_cmd::flags::alt2);
	LUA_DEF_G_INT_("IN_SCORE",		c_user_cmd::flags::score);
	LUA_DEF_G_INT_("IN_SPEED",		c_user_cmd::flags::speed);
	LUA_DEF_G_INT_("IN_WALK",		c_user_cmd::flags::walk);
	LUA_DEF_G_INT_("IN_ZOOM",		c_user_cmd::flags::zoom);
	LUA_DEF_G_INT_("IN_WEAPON1",	c_user_cmd::flags::weapon1);
	LUA_DEF_G_INT_("IN_WEAPON2",	c_user_cmd::flags::weapon2);
	LUA_DEF_G_INT_("IN_BULLRUSH",	c_user_cmd::flags::bull_rush);
	LUA_DEF_G_INT_("IN_GRENADE1",	c_user_cmd::flags::grenade1);
	LUA_DEF_G_INT_("IN_GRENADE2",	c_user_cmd::flags::grenade2);

}

// execute a lua string and handle errors correctly
void LuaSystem::RunString(std::string str) {
	bool bErrorCaused = luaL_dostring(L, str.c_str());

	if (bErrorCaused) {
		CONSOLE->AddToBuffer(lua_tostring(L, -1));
	}
}

// lauxlib.h references @ line 221
void Lua_Print(const char* str) {
	CONSOLE->AddToBuffer(str);
}

// config.ChangeFOV(fov, viewmodel_fov)
LUA_C_LIBFUNC(config, ChangeFOV) {
	LUA_ARG_CHECK(2);

	double arg0 = lua_tonumber(L, 1);
	double arg1 = lua_tonumber(L, 2);

	if (arg0 > 180 || arg0 < 0) {
		luaL_error(L, _("arg0 out of bounds"));
		return 0;
	}

	if (arg1 > 180 || arg1 < 0) {
		luaL_error(L, _("arg1 out of bounds"));
		return 0;
	}

	config.misc.fov			= arg0;
	config.misc.fov_view	= arg1;

	return 0;
}

// config.ChangeBhop(enabled)
LUA_C_LIBFUNC(config, ChangeBhop) {
	LUA_ARG_CHECK(1);

	bool arg0 = lua_toboolean(L, 1);

	config.misc.knife = arg0;
}

// Color(r, g, b)
LUA_C_FUNC(Color) {
	LUA_ARG_CHECK(3);

	int red			= lua_tointeger(L, 1);
	int green		= lua_tointeger(L, 2);
	int blue		= lua_tointeger(L, 3);

	red		< 0 ? red	= 0 : (red		> 255 ? red		= 255 : red		= red);
	green	< 0 ? green = 0 : (green	> 255 ? green	= 255 : green	= green);
	blue	< 0 ? blue	= 0 : (blue		> 255 ? blue	= 255 : blue	= blue);

	lua_newtable(L);

	lua_pushinteger(L, red);
	lua_setfield(L, -2, _("r"));

	lua_pushinteger(L, green);
	lua_setfield(L, -2, _("g"));

	lua_pushinteger(L, blue);
	lua_setfield(L, -2, _("b"));

	return 1;
}

// yes im using imgui color for lua color no i do not care
ImColor LUA_ARG_CLR_(lua_State* L) {
	lua_getfield(L, 1, "r");
	lua_getfield(L, 1, "g");
	lua_getfield(L, 1, "b");

	int r	= luaL_checkinteger(L, -3);
	int g	= luaL_checkinteger(L, -2);
	int b	= luaL_checkinteger(L, -1);

	// default 255 for now. alpha won't be needed (yet)
	return ImColor(r, g, b, 255);
}

// draw.Text(clr, str, x, y)
LUA_C_LIBFUNC(draw, Text) {
	LUA_ARG_CHECK(4);

	auto clr	= LUA_ARG_CLR(L);
	auto str	= luaL_checkstring(L, 2);
	auto x		= luaL_checkinteger(L, 3);
	auto y		= luaL_checkinteger(L, 4);

	renderer->text(c_vector2d(x, y), str, IMVEC4_TO_C_COLOR(clr));

	return 0;
}

// draw.Line(clr, x1, y1, x2, y2)
LUA_C_LIBFUNC(draw, Line) {
	LUA_ARG_CHECK(5);

	auto clr = LUA_ARG_CLR(L);
	auto x1 = luaL_checkinteger(L, 2);
	auto y1 = luaL_checkinteger(L, 3);
	auto x2 = luaL_checkinteger(L, 4);
	auto y2 = luaL_checkinteger(L, 5);

	renderer->line(c_vector2d(x1, y1), c_vector2d(x2, y2), IMVEC4_TO_C_COLOR(clr));

	return 0;
}

// draw.Rect(clr, x, y, w, h)
LUA_C_LIBFUNC(draw, Rect) {
	LUA_ARG_CHECK(5);

	auto clr = LUA_ARG_CLR(L);
	auto x1 = luaL_checkinteger(L, 2);
	auto y1 = luaL_checkinteger(L, 3);
	auto x2 = luaL_checkinteger(L, 4);
	auto y2 = luaL_checkinteger(L, 5);

	renderer->rect(c_vector2d(x1, y1), c_vector2d(x2, y2), IMVEC4_TO_C_COLOR(clr));

	return 0;
}

// draw.RectFilled(clr, x, y, w, h)
LUA_C_LIBFUNC(draw, RectFilled) {
	LUA_ARG_CHECK(5);

	auto clr = LUA_ARG_CLR(L);
	auto x1 = luaL_checkinteger(L, 2);
	auto y1 = luaL_checkinteger(L, 3);
	auto x2 = luaL_checkinteger(L, 4);
	auto y2 = luaL_checkinteger(L, 5);

	renderer->rect_filled(c_vector2d(x1, y1), c_vector2d(x2, y2), IMVEC4_TO_C_COLOR(clr));

	return 0;
}

// menu.Start(name, width, height)
LUA_C_LIBFUNC(menu, Start) {
	LUA_ARG_CHECK(3);

	auto name	= luaL_checkstring(L, 1);
	auto width	= luaL_checkinteger(L, 2);
	auto height = luaL_checkinteger(L, 3);

	ImGui::SetNextWindowSize(ImVec2(width, height));
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	return 0;
}

// menu.End()
LUA_C_LIBFUNC(menu, End) {
	ImGui::End();

	return 0;
}

// menu.Text(str)
LUA_C_LIBFUNC(menu, Text) {
	LUA_ARG_CHECK(1);

	auto str = luaL_checkstring(L, 1);

	ImGui::Text(str);

	return 0;
}

// menu.TextColored(clr, text)
LUA_C_LIBFUNC(menu, TextColored) {
	LUA_ARG_CHECK(2);

	auto clr = LUA_ARG_CLR(L);
	auto str = luaL_checkstring(L, 2);

	ImGui::TextColored((ImVec4)clr, str);

	return 0;
}

LUA_C_LIBFUNC(game, IsInGame) {
	LUA_ARG_CHECK(0);

	lua_pushboolean(L, engine_client()->is_ingame());

	return 1; // boolean
}

LUA_C_LIBFUNC(game, GetPlayers) {
	LUA_ARG_CHECK(0);

	lua_newtable(L);

	int j = 1;
	for (int i = 0; i < engine_client()->get_max_clients(); i++) {
		auto pEnt = (c_base_player*)client_entity_list()->get_client_entity(i);

		if (!pEnt)
			continue;

		lua_pushnumber(L, i);		// push ent id
		lua_rawseti(L, -2, j);		// set it in the array

		j++;
	}

	return 1; // { 1, 2, 3, 4, 5, 6, 7 .. }
}

LUA_C_LIBFUNC(game, LocalPlayer) {
	LUA_ARG_CHECK(0);

	if (!engine_client()->is_ingame()) {
		lua_pushinteger(L, -1);
	}
	else {
		lua_pushinteger(L, engine_client()->is_ingame());
	}

	return 1; // int
}

LUA_C_LIBFUNC(player, IsAlive) {
	LUA_ARG_CHECK(1);

	auto pEnt = (c_base_player*)client_entity_list()->get_client_entity(luaL_checkinteger(L, 1));

	if (!pEnt) {
		lua_pushboolean(L, false);
	}
	else {
		lua_pushboolean(L,  pEnt->is_alive());
	}

	return 1; // boolean
}

LUA_C_LIBFUNC(player, IsDormant) {
	LUA_ARG_CHECK(1);

	auto pEnt = (c_base_player*)client_entity_list()->get_client_entity(luaL_checkinteger(L, 1));

	if (!pEnt) {
		lua_pushboolean(L, false);
	}
	else {
		lua_pushboolean(L, pEnt->is_dormant());
	}

	return 1; // boolean
}

LUA_C_LIBFUNC(player, GetName) {

	auto pEnt = (c_base_player*)client_entity_list()->get_client_entity(luaL_checkinteger(L, 1));

	if (!pEnt) {
		lua_pushstring(L, "<invalid>");
	}
	else {
		player_info info;
		engine_client()->get_player_info(luaL_checkinteger(L, 1), &info);
		lua_pushstring(L, info.name);
	}

	return 1; // string
}

LUA_C_LIBFUNC(player, GetScreenPos) {
	
	return 1; // { x, y }
}

/*

Hooks system, don't touch it works too well and im not sure why

*/


class Lua_Hook {
public:

	std::string type;
	std::string name;
	int index;

	Lua_Hook(std::string type, std::string name, int index) {
		this->type = type;
		this->name = name;
		this->index = index;
	}
};

std::vector<Lua_Hook*> g_Hooks;

LUA_C_LIBFUNC(hook, Add) {
	LUA_ARG_CHECK(3);

	auto type = lua_tostring(L, 1);
	auto name = lua_tostring(L, 2);
	auto inde = luaL_ref(L, LUA_REGISTRYINDEX);

	g_Hooks.push_back(new Lua_Hook(type, name, inde));
}

LUA_C_LIBFUNC(hook, Remove) {
	LUA_ARG_CHECK(2);

	auto type = lua_tostring(L, 1);
	auto name = lua_tostring(L, 2);

	for (auto Hook : g_Hooks) {
		if ((strcmp(Hook->name.c_str(), name) == 0) && (strcmp(Hook->type.c_str(), type) == 0)) {
			// fuck the std lib
			// https://stackoverflow.com/questions/26567687/how-to-erase-vector-element-by-pointer
			g_Hooks.erase(std::remove(g_Hooks.begin(), g_Hooks.end(), Hook), g_Hooks.end()); 
		}
	}
}

void LuaSystem::ExecuteHook(std::string type) {
	for (auto Hook : g_Hooks) {
		if (strcmp(Hook->type.c_str(), type.c_str()) == 0) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, Hook->index);
			auto x = lua_pcall(L, 0, 0, 0);

			// if x is 0 then there's no errors
			if (x) { 
				CONSOLE->AddToBuffer(lua_tostring(L, -1));
			}
		}
	}
}


// this hook should be called after all the current aimbot etc, this allows for custom antiaim (if that is even needed?) etc
// TODO: fix lmao
void LuaSystem::ExecuteCreateMove(c_user_cmd* cmd) {
	for (auto Hook : g_Hooks) {
		if (strcmp(Hook->type.c_str(), "CreateMove") != 0) {
			continue;
		}

		lua_rawgeti(L, LUA_REGISTRYINDEX, Hook->index);

		// pack the table as the argument for CreateMove
		lua_newtable(L);
		LUA_CM_INTEGER(command_number);
		LUA_CM_INTEGER(tick_count);
		LUA_CM_VECTOR(viewangles);
		LUA_CM_VECTOR(aim_direction);
		LUA_CM_FLOAT(forwardmove);
		LUA_CM_FLOAT(sidemove);
		LUA_CM_FLOAT(upmove);
		LUA_CM_INTEGER(buttons);
		LUA_CM_INTEGER(impulse);
		LUA_CM_INTEGER(weapon_select);
		LUA_CM_INTEGER(weapon_select_subtype);
		LUA_CM_INTEGER(random_seed);
		LUA_CM_INTEGER(mousedx);
		LUA_CM_INTEGER(mousedy);
		LUA_CM_BOOLEAN(predicted);

		// 1 arg, 1 result, no error func
		auto errorcode = lua_pcall(L, 1, 1, 0);

		if (errorcode) {
			CONSOLE->AddToBuffer(lua_tostring(L, -1));
			return;
		}

		// unpack the returned cmd table by pushing it backwards onto the stack ..
 		lua_getfield(L, 1, "command_number");				// -19
		lua_getfield(L, 1, "tick_count");					// -18
		lua_getfield(L, 1, "viewangles_x");					// -17
		lua_getfield(L, 1, "viewangles_y");					// -16
		lua_getfield(L, 1, "viewangles_z");					// -15
		lua_getfield(L, 1, "aim_direction_x");				// -14
		lua_getfield(L, 1, "aim_direction_y");				// -13
		lua_getfield(L, 1, "aim_direction_z");				// -12
		lua_getfield(L, 1, "forwardmove");					// -11
		lua_getfield(L, 1, "sidemove");						// -10
		lua_getfield(L, 1, "upmove");						// -9
		lua_getfield(L, 1, "buttons");						// -8
		lua_getfield(L, 1, "impulse");						// -7
		lua_getfield(L, 1, "weapon_select");				// -6
		lua_getfield(L, 1, "weapon_select_subtype");		// -5
		lua_getfield(L, 1, "random_seed");					// -4
		lua_getfield(L, 1, "mousedx");						// -3
		lua_getfield(L, 1, "mousedy");						// -2
		lua_getfield(L, 1, "predicted");					// -1


		// .. and then reading each fucking value directly. there's probably a better way of doing this but it's too late now
		cmd->command_number				= lua_tointeger(L, -19);
		cmd->tick_count					= lua_tointeger(L, -18);
		cmd->viewangles					= c_vector3d(
										  (float)lua_tonumber(L, -17),
										  (float)lua_tonumber(L, -16),
										  (float)lua_tonumber(L, -15));
		cmd->aim_direction				= c_vector3d(
										  (float)lua_tonumber(L, -14),
										  (float)lua_tonumber(L, -13),
										  (float)lua_tonumber(L, -12));
		cmd->forwardmove				= (float)lua_tonumber(L, -11);
		cmd->sidemove					= (float)lua_tonumber(L, -10);
		cmd->upmove						= (float)lua_tonumber(L, -9);
		cmd->buttons					= lua_tointeger(L, -8);
		cmd->impulse					= lua_tointeger(L, -7);
		cmd->weapon_select				= lua_tointeger(L, -6);
		cmd->weapon_select_subtype		= lua_tointeger(L, -5);
		cmd->random_seed				= lua_tointeger(L, -4);
		cmd->mousedx					= lua_tointeger(L, -3);
		cmd->mousedy					= lua_tointeger(L, -2);
		cmd->predicted					= lua_toboolean(L, -1);

		return;
	}
}

void LuaSystem::ClearHooks() {
	for (auto Hook : g_Hooks) {
		g_Hooks.erase(std::remove(g_Hooks.begin(), g_Hooks.end(), Hook), g_Hooks.end());
	}
}