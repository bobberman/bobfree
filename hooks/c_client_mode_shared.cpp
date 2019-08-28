#include "c_client_mode_shared.h"
#include "c_net_channel_.h"
#include "../sdk/c_input.h"
#include "../hacks/c_miscellaneous.h"
#include "../hacks/c_legitbot.h"
#include "../hacks/c_antiaim.h"

void c_client_mode_shared::hook()
{
	static c_hook<uintptr_t> hook(client_mode());
	_override_view = hook.apply<override_view_t>(18, override_view);
	//_override_mouse_input = hook.apply<override_mouse_input_t>(23, override_mouse_input);
	_should_draw_fog_t = hook.apply<should_draw_fog_t>(17, should_draw_fog);
	//_should_draw_local_player_t = hook.apply<should_draw_local_player_t>(14, should_draw_local_player);

}

uintptr_t* c_client_mode_shared::client_mode()
{
	static auto client_mode = **reinterpret_cast<uintptr_t***>((*reinterpret_cast<uintptr_t**>(base_client()))[10] + 5);
	return client_mode;
}

bool c_client_mode_shared::should_draw_local_player(uintptr_t* client_mode, c_base_entity* entity)
{
	const auto local_player = c_cs_player::get_local_player();

	if (!engine_client()->is_ingame())
		return _should_draw_local_player_t(client_mode, entity);

	if (entity == local_player && input->camera_in_third_person && config.rage.antiaim_settings.enabled) // we want to smooth player out if anti aiming
		return false;
	else
		return true;
}

bool c_client_mode_shared::should_draw_fog(uintptr_t* client_mode)
{
	if (!engine_client()->is_ingame())
		return _should_draw_fog_t(client_mode);

	if (config.misc.no_fog)
			return false;
	else
		return _should_draw_fog_t(client_mode);
}

void c_client_mode_shared::override_view(uintptr_t* client_mode, uint32_t,  c_view_setup* view_setup)
{
	if (!engine_client()->is_ingame())
		return _override_view(client_mode, view_setup);

	const auto local = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_local_player()));

	if (local && local->is_alive() && local->is_scoped() && config.misc.remove_zoom)
		view_setup->fov = 90;

	if (antiaim->is_fakeducking && input->camera_in_third_person)//testing if this even works
		view_setup->origin.z = local->get_abs_origin().z + 64.f;

	c_miscellaneous::set_camera_to_thirdperson();

	_override_view(client_mode, view_setup);

	c_miscellaneous::disable_post_processing();
}

void c_client_mode_shared::override_mouse_input(uintptr_t* client_mode, uint32_t, float* x, float* y)
{
	//if (engine_client()->is_ingame())
		//c_legitbot::aim(x, y);

	_override_mouse_input(client_mode, x, y);
}
