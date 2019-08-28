#pragma once

#include "macros.h"

class c_cs_game_rules_proxy_proxy
{
public:
	static c_cs_game_rules_proxy_proxy* get()
	{
		static const auto proxy = *reinterpret_cast<c_cs_game_rules_proxy_proxy***>(
			sig("client_panorama.dll", "83 3d ? ? ? ? 0 74 ? A1 ? ? ? ? B9 ? ? ? ? FF 50 ? 85 C0 75 ? 8B 0d ? ? ? ?") + 0x2);

		return *proxy;
	}

	offset(is_freeze_period(), bool, 0x20)
	offset(is_valve_ds(), bool, 0x75)
	offset(is_bomb_planed(), bool, 0x8D1)
	offset(is_bomb_dropped(), bool, 0x8D0)

	vfunc(28, should_collide(const int group, const int group_alt), bool(__thiscall*)(void*, int, int))(group, group_alt)
};

#define game_rules c_cs_game_rules_proxy_proxy::get()