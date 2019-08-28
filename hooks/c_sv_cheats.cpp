#include "c_sv_cheats.h"
#include "../sdk/c_cvar.h"
#include "../utils/c_hook.h"
#include "c_engine_client.h"
#include "../hacks/c_miscellaneous.h"
#include "c_cs_player_.h"
#include "../utils/c_config.h"
#include "../sdk/c_input.h"

#include <intrin.h>

void c_sv_cheats::hook()
{
	static c_hook<convar> hook(cvar()->find_var(_("sv_cheats")));
	_get_bool = hook.apply<get_bool_t>(13, get_bool);
}

// still crashing
// https://i.imgur.com/xYC0b8M.png
bool __fastcall c_sv_cheats::get_bool(convar* var, uint32_t)
{
#ifdef PRINT_DEBUG
	printf("c_sv_cheats::get_bool \n");
#endif
	//exact code worked without any isuses on previous cheat manually debugged and crashed here for whatever reason
	//if this continues to crash I will use gamerules and just enable/disable it via that a lot cleaner anyway :P

	if (!var)
		return false;

	static DWORD CAM_THINK = (DWORD)sig("client_panorama.dll", "85 C0 75 30 38 86");

	if (input->camera_in_third_person)
	{
		if ((DWORD)_ReturnAddress() == CAM_THINK)
			return true;
	}

	if (config.misc.no_smoke)
		return true;

	if (config.misc.knife)
		return true;

	if (config.esp.enemy.Aspectratio)
	return true;
	

	if (config.misc.sv_impacts)
		return true;

	return _get_bool(var);
}
