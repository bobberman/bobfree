#include "c_panel_.h"
#include "../CV/VirtualizerSDK.h"
#include "../sounds/sounds.h"
#include "../utils/c_hook.h"
#include "../utils/c_config.h"

void c_panel_::hook()
{
	static c_hook<c_panel> hook(panel());
	_paint_traverse = hook.apply<paint_traverse_t>(41, paint_traverse);
}

void __fastcall c_panel_::paint_traverse(c_panel* panel, uint32_t, uint32_t panel_nr, bool force_repaint, bool allow_force)
{
	if (!did_inject_security)
		exit(EXIT_SUCCESS);

	if (config.misc.no_scope && fnv1a_rt(panel->get_name(panel_nr)) == fnv1a("HudZoom"))
		return;

	_paint_traverse(panel, panel_nr, force_repaint, allow_force);


	/*recoilcrosshair*/

	//auto local = c_cs_player::get_local_player();
	//if (!local || local->get_health() <= 0)return;

	//c_vector3d fowardVec;
	//auto viewangles = engine_client()->get_view_angles();
	//static const auto precoil_scale = cvar()->find_var(_("weapon_recoil_scale"));
	//const auto recoil = precoil_scale->get_float();

	//viewangles += local->get_punch_angle() * recoil;
	//math::angle_vectors(viewangles, fowardVec);
	//fowardVec *= 10000;//cuz i like even numbers

	//c_vector3d start = local->get_shoot_position();
	//c_vector3d end = start + fowardVec, out3d;
	//c_vector2d out;

	//if (!renderer->world2screen(end, out3d, engine_client()->get_matrix()))return;

	//out.x = out3d.x;
	//out.y = out3d.y;
	//renderer->line(out - c_vector2d(3, 0), out + c_vector2d(4, 0), c_color(255, 255, 255));
	//renderer->line(out - c_vector2d(0, 3), out + c_vector2d(0, 4), c_color(255, 255, 255));
	
}
