#pragma once

#include "../includes.h"
#include "../sdk/misc.h"

class c_client_mode_shared
{
	typedef void(__thiscall* override_view_t)(uintptr_t*, c_view_setup*);
	typedef void(__thiscall* override_mouse_input_t)(uintptr_t*, float*, float*);
	typedef bool(__thiscall* should_draw_fog_t)(uintptr_t*);
	typedef bool(__thiscall* should_draw_local_player_t)(uintptr_t*, c_base_entity* entity);


public:
	static void hook();

	static uintptr_t* client_mode();
private:
	inline static override_view_t _override_view;
	inline static override_mouse_input_t _override_mouse_input;
	inline static should_draw_fog_t _should_draw_fog_t;
	inline static should_draw_local_player_t _should_draw_local_player_t;

	static void __fastcall override_view(uintptr_t* client_mode, uint32_t, c_view_setup* view_setup);
	static void __fastcall override_mouse_input(uintptr_t* client_mode, uint32_t, float* x, float* y);
	static bool __fastcall should_draw_fog(uintptr_t* client_mode);
	static bool __fastcall should_draw_local_player(uintptr_t* client_mode, c_base_entity* local);

};
