#include "../BASS/bass.h"
#include "c_indicators.h"
#include "../hacks/c_antiaim.h"
#include "../hacks/c_animation_system.h"
#include "../sdk/c_engine_client.h"
#include "../sdk/c_engine_trace.h"
#include "../sdk/c_weapon_system.h"
#include "../hooks/idirect3ddevice9.h"
#include "../hacks/c_antiaim.h"
#include "../hacks/c_fake_ping.h"
#include "../sdk/c_client_state.h"
#include "../hacks/c_esp.h"
#include "../utils/math.h"
#include "../hacks/c_trace_system.h"
#include "../sdk/c_input.h"
#include "../sdk/c_debug_overlay.h"
#include "../hacks/c_aimhelper.h"




void pen_crosshair()
{
	return;
	const auto local = c_cs_player::get_local_player();

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return;

	const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!info)
		return;

	float length = info->flRange;

	game_trace tr;
	ray r;
	c_trace_filter filter;
	filter.skip_entity = local;

	c_vector3d start = local->get_shoot_position();
	c_vector3d viewangles = engine_client()->get_view_angles();
	c_vector3d fw;

	math::angle_vectors(viewangles, fw);
	fw.normalize();

	c_vector3d end = start + fw * length;

	r.init(start, end);

	engine_trace()->trace_ray(r, mask_shot, &filter, &tr);

	if (!tr.did_hit() || tr.entity)
	{
		int width = 0;
		int height = 0;
		engine_client()->get_screen_size(width, height);

		bool damage = false;

		auto const wall = trace_system->wall_penetration(local->get_shoot_position(), tr.endpos, nullptr, local);

		if (wall.has_value())
			damage = true;

		renderer->circle_filled(c_vector2d(width / 2, height / 2), 2.f, damage ? c_color(0, 150, 0, 255) : c_color(255, 0, 0, 255));
	}
}


float lerp(float t, float a, float b) {
	return (1 - t)*a + t * b;
}

// speed
const float ind_speed = 0.1f;



int GetMoveTypes(c_cs_player* local) {
	auto anim_state = local->get_anim_state();
	bool is_local_moving = anim_state->velocity.length2d() > 0.1 && local->get_flags() & c_base_player::flags::on_ground && !(GetAsyncKeyState(config.rage.slow_walk));
	bool is_local_standing = anim_state->velocity.length2d() == 0 && local->get_flags() & c_base_player::flags::on_ground && !(GetAsyncKeyState(config.rage.slow_walk));
	bool is_local_air = !(local->get_flags() & c_base_player::flags::on_ground) && !(GetAsyncKeyState(config.rage.slow_walk));
	bool is_local_slow = (local->get_flags() & c_base_player::flags::on_ground) && (GetAsyncKeyState(config.rage.slow_walk));

	if (is_local_standing) return 0;
	if (is_local_moving) return 1;
	if (is_local_air) return 2;
	if (is_local_slow) return 3;
}

void c_indicators::draw()
{
	const auto local = c_cs_player::get_local_player();

	if (!engine_client()->is_ingame() || !local || !local->is_alive())
		return;

	if (!config.misc.indicators)
		return;

	

	auto position = c_vector2d(0, 500);
	auto positionw = c_vector2d(10, 100);

	if (local->is_alive()) {

		int mtype = GetMoveTypes(local);
		int type = 0;
		if (mtype == 2) type = config.rage.antiaim_settings.yaw_air_mode;
		else if (mtype == 1) type = config.rage.antiaim_settings.yaw_moving_mode;
		else if (mtype == 0) type = config.rage.antiaim_settings.yaw_mode;
		else if (mtype == 3) type = config.rage.antiaim_settings.yaw_slow_mode;

		if (config.rage.enabled) {
			const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));
		
			if(weapon){
			auto cfg = c_aimhelper::get_weapon_conf();
			{
				static float value = 0.f;
				float newvalue = GetAsyncKeyState(cfg->head_aim_key) ? 1.f : 0.f;
				value = lerp(ind_speed, value, newvalue);
				auto c = config.misc.theme;
				if (value > 0.1f)
					draw_progressbar(position, "FORCE HEAD", c, value);
			}
			{
				static float value = 0.f;
				float newvalue = GetAsyncKeyState(cfg->body_aim_key) ? 1.f : 0.f;
				value = lerp(ind_speed, value, newvalue);
				auto c = config.misc.theme;
				if (value > 0.1f)
					draw_progressbar(position, "FORCE BODY", c, value);
			}
			}
		}

		if(config.rage.antiaim_settings.enabled){
		{
			static float value = 0.f;
			float newvalue = GetAsyncKeyState(config.rage.slow_walk) ? 1.f : 0.f;
			value = lerp(ind_speed, value, newvalue);
			auto c = config.misc.theme;
			if(value > 0.1f)
				draw_progressbar(position, "SLOWWALK", c, value);
		}

		{
			static float value = 0.f;
			float newvalue = GetAsyncKeyState(config.rage.fake_duck) ? 1.f : 0.f;
			value = lerp(ind_speed, value, newvalue);
			auto c = config.misc.theme;
			if (value > 0.1f)
				draw_progressbar(position, "BOBDUCK", c, value);
		}

		{
			static float value = 0.f;
			float newvalue = GetAsyncKeyState(config.misc.inverter) ? 1.f : 0.f;
			value = lerp(ind_speed, value, newvalue);
			auto c = config.misc.theme;
			if (value > 0.1f)
				draw_progressbar(position, "INVERT", c, value);
		}
		{
			draw_watermark(positionw, "bobhack alpha");
		}

		}

		if(type==4){
			static float value = 0.f;
			float newvalue = GetAsyncKeyState(config.misc.flip) ? 1.f : 0.f;
			value = lerp(ind_speed, value, newvalue);
			auto c = config.misc.theme;
			if (value > 0.1f)
				draw_progressbar(position, "SWITCH", c, value);
		}

		{
			static float value = 0.f;
			float newvalue = antiaim->estimated_choke / 16.f;
			value = lerp(ind_speed, value, newvalue);
			draw_progressbar(position, "CHOKE", c_color(0, 255, 255), value);
		}

		
		{
			static float value = 0.f;
			static auto maxvel = cvar()->find_var("sv_maxvelocity");
			float newvalue = std::clamp(local->get_velocity().length2d(), 0.f, 300.f) / 300.f; /* FOR NOW cause more retarded servers do max vel at 3500*/
			value = lerp(ind_speed, value, newvalue);
			draw_progressbar(position, "VELOCITY", c_color(255, 0, 255), value);
		}


		if (config.misc.indicator_antiaim ) {
			
			if(type == 4)
			{
				auto color = config.misc.indicator_antiaimc;
				renderer->text(renderer->get_center() - c_vector2d(75 + renderer->get_text_size("left", fnv1a("choke_font")).x, 0.f), "left", c_color(color.red, color.green, color.blue, c_antiaim::GetDirection() == 0 ? 200 : 100), fnv1a("choke_font"), c_font::font_flags::centered_x | c_font::font_flags::centered_y | c_font::font_flags::drop_shadow);
				renderer->text(renderer->get_center() + c_vector2d(75, 0.f), "right", c_color(color.red, color.green, color.blue, c_antiaim::GetDirection() == 2 ? 200 : 100), fnv1a("choke_font"), c_font::font_flags::centered_x | c_font::font_flags::centered_y | c_font::font_flags::drop_shadow);
				
			
			}
		}
		
	}
	//draw_ping_acceptance(position);

	//pen_crosshair();

}

void c_indicators::draw_ping_acceptance(c_vector2d& position)
{
	const auto wanted_ping = fake_ping->calculate_wanted_ping(net_channel) * 1000.f;

	if (wanted_ping > 0.f)
	{
		static const auto ping_acceptance = __("Ping: %d");

		_rt(pa_format, ping_acceptance);
		char pa[40];
		sprintf_s(pa, pa_format, static_cast<int>(wanted_ping));
		renderer->text(position, pa, c_color::gradient2(), fnv1a("choke_font"), c_font::font_flags::drop_shadow);
	}
}

void c_indicators::draw_radio_info(c_vector2d& position)
{
	if (strlen(BASS::bass_metadata) > 0 && config.misc.radio_channel)
	{
		static const auto radio_info = __("Now playing:");
		static const auto muted = __("MUTED");

		_rt(radio, radio_info);
		renderer->text(position, radio, c_color::primary(), fnv1a("pro13"), c_font::font_flags::drop_shadow);
		position.y += 16;

		renderer->text(position, BASS::bass_metadata, c_color::foreground(), fnv1a("pro13"), c_font::font_flags::drop_shadow);
		position.y += 16;

		if (radio_muted)
		{
			_rt(rad_muted, muted);
			renderer->text(position, rad_muted, c_color::gradient3(), fnv1a("pro13"), c_font::font_flags::drop_shadow);
		}
	}

	position.y += 32;
}

void c_indicators::draw_choke(c_vector2d& position)
{
	if (!config.rage.enabled)
		return;

	static const auto choke_display = __("choke: %i");
	_rt(hit, choke_display);

	char msg[255];
	sprintf_s(msg, hit, antiaim->estimated_choke);

	renderer->text(position, msg, c_color::gradient1(), fnv1a("choke_font"), c_font::font_flags::drop_shadow);
	position.y += 16;
}

void c_indicators::draw_antiaim(const char* text, float angle, c_color color, c_cs_player* local)
{
	c_vector2d src, dst;
	c_vector3d dst3D, src3D, forward;

	game_trace tr;
	ray r;
	c_trace_filter filter;

	filter.skip_entity = local;

	math::angle_vectors(c_qangle(0, angle, 0), forward);
	src3D = local->get_origin();
	dst3D = src3D + (forward * 50.f);

	r.init(src3D, dst3D);

	engine_trace()->trace_ray(r, 0, &filter, &tr);

	debug_overlay()->add_line_overlay(src3D, tr.endpos, 255, 0, 0, 1, global_vars_base->interval_per_tick * 2);

}

void c_indicators::draw_progressbar(c_vector2d& position, const char* name, const c_color color, const float progress)
{

	auto textpos = renderer->get_text_size(name, fnv1a("menu_font"));
	const auto size = c_vector2d(textpos.x + 8, 1);
	renderer->rect_filled(position, c_vector2d(4 + textpos.x + 4, 17), c_color(0,0,0,175));
	renderer->rect_filled(position, c_vector2d(4, 17), color);
	renderer->text(position + c_vector2d(6, 2), name, c_color::foreground(), fnv1a("menu_font"), c_font::font_flags::drop_shadow); //text

	renderer->rect_filled(position + c_vector2d(2, 16), c_vector2d(std::clamp(size.x * std::clamp(0.f + progress, 0.f, 1.f), 0.f, size.x), size.y), color); // bar
	position.y += 21;
}
void c_indicators::draw_watermark(c_vector2d& position, const char* name) {
	auto textpos = renderer->get_text_size(name, fnv1a("menu_font"));
	const auto size = c_vector2d(textpos.x + 8, 1);
	renderer->text(position + c_vector2d(6, 2), name, c_color::foreground(), fnv1a("menu_font"), c_font::font_flags::drop_shadow);
}
void c_indicators::draw_arrow(float angle, c_color color)
{
	c_vector2d pos[8] =
	{
		{ -7.f, -50.f },
		{ -7.f, -140.f },

		{ 7.f, -50.f },
		{ 7.f, -140.f },

		{ -20.f, -130.f },
		{ 0.f, -160.f },

		{ 20.f, -130.f },
		{ 0.f, -160.f }
	};
	
	for (auto& p : pos)
	{
		const auto s = sin(angle);
		const auto c = cos(angle);

		p = c_vector2d(p.x * c - p.y * s, p.x * s + p.y * c) + renderer->get_center();
	}

	renderer->line(pos[0], pos[1], color);
	renderer->line(pos[2], pos[3], color);
	renderer->line(pos[4], pos[5], color);
	renderer->line(pos[6], pos[7], color);
}
