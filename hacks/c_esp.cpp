#include "c_esp.h"
#include "c_aimhelper.h"
#include "c_trace_system.h"

#include "c_chams.h"

#include "../utils/math.h"
#include "../sdk/c_model_info_client.h"
#include "../utils/c_config.h"
#include "../sdk/c_weapon_system.h"
#include "../sdk/c_view_beams.h"
#include "../sdk/c_debug_overlay.h"
#include <d3d9.h>
#include "../imgui/imgui.h"
#include "../ImGUI/DX9/imgui_impl_dx9.h"
#include "../imgui/imgui_internal.h"
#include "../menu/c_menu.h"


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

float esp_alpha_fade[64];
float plerp(float t, float a, float b) {
	return (1 - t)*a + t * b;
}
void c_esp::draw()
{
	if (!engine_client()->is_ingame())
	{
		if (!info.players.empty() || info.players.size() > 0)
			info.players.clear();

		return;
	}

	//std::scoped_lock<std::mutex> lock(info.mutex);

	draw_dmg();
	draw_players();
	draw_nades();
	draw_debug();
	draw_scope();
	
}

void c_esp::draw_local_impact(c_vector3d start, c_vector3d end)
{ 
	if (!config.esp.local_impact)
		return;

	start.z *= 1.05f;

	draw_impact(start, end, config.esp.local_impacts_color);
}

void c_esp::draw_spectators() // wip
{
	const auto local = c_cs_player::get_local_player();

	if (!local)
		return;
}

void c_esp::pen_crosshair()
{

}

void c_esp::draw_enemy_impact(c_game_event* event)
{
	/*

	if (!config.esp.enemy.impacts)
		return;

	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);
	const auto player = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(index));

	if (player && player->is_enemy() && player->is_alive()
		&& !player->is_dormant() && config.esp.enemy.impacts)
		draw_impact(player->get_shoot_position(), { event->get_float(_("x")),
			event->get_float(_("y")),
			event->get_float(_("z")) }, config.esp.impacts_color);
	*/
}

bool c_esp::draw_valid_visual_player(c_cs_player* e, bool check_team, bool check_dormant)
{
	if (!e) { return false; }

	if (e->is_dormant() && check_dormant) { return false; }

	if (!e->is_alive()) { return false; }

	if (!e->is_player()) { return false; }

	return true;
}

void c_esp::store_data()
{
	info.players.clear();
	info.players.reserve(64);

	if (!engine_client()->is_ingame())
		return;

	//std::scoped_lock<std::mutex> lock(info.mutex);

	info.world_to_screen_matrix = renderer->world_to_screen_matrix();

	static bool is_esp_checked = true;

	client_entity_list()->for_each_player([&] (c_cs_player* player) -> void
	{
		if (!draw_valid_visual_player(player, false, !is_esp_checked))
			return;

		const auto anim_info = animation_system->get_animation_info(player);

		player_info current;
		current.handle = player->get_handle();
		current.player = player;
		current.index = player->index();
		
		memcpy(current.name, player->get_info().name, 15);
		current.name[15] = '\0';
		current.origin = player->get_abs_origin();
		current.angles = player->get_abs_angles();
		current.duck_amount = player->get_duck_amount();
		current.health = std::clamp(player->get_health(), 0, 100);
		current.is_enemy = player->is_enemy();
		current.is_on_ground = player->is_on_ground();
		current.dormant = player->is_dormant();
		current.layers = *player->get_animation_layers();
		current.reload = player->get_sequence_activity(current.layers[1].sequence) == act_csgo_reload;
		current.good_bones = player->setup_bones(current.bones, 128, bone_used_by_anything, global_vars_base->curtime);
		current.head_position = player->get_hitbox_position(c_cs_player::hitbox::head, current.bones);
		
		current.b_kevlar = player->get_armor() > 0;
		current.b_helmet = player->has_helmet();
		current.b_defusing = player->get_defusing();
		current.b_scoped = player->is_scoped();

		if (anim_info)
		{
			current.shots_missed = anim_info->missed_due_to_resolver + anim_info->missed_due_to_spread;
			
			switch (anim_info->brute_state)
			{
			case resolver_start:
				current.resolver_mode = 0;
				break;
			case resolver_inverse:
				current.resolver_mode = 1;
				break;
			case resolver_no_desync:
				current.resolver_mode = 2;
				break;
			default:
			case resolver_jitter:
				current.resolver_mode = 3;
				break;
			}
		}

		const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(player->get_current_weapon_handle()));

		if (weapon)
		{
			const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());
			current.ammo = weapon->get_current_clip();
			current.ammo_max = info->iMaxClip1;
			current.pWeapon = weapon;
		}
		else
		{
			current.pWeapon = nullptr;
			current.ammo = 0;
			current.ammo_max = 1;
		}

		info.players.push_back(current);
	});

	info.nades.clear();

	client_entity_list()->for_each([](c_client_entity* entity) -> void {
		static const auto is_nade_projectile = [](uint32_t id) {
			switch (id) {
			case cbasecsgrenadeprojectile:
			case cdecoyprojectile:
			case csmokegrenadeprojectile:
			case cmolotovprojectile:
				return true;
			default:
				return false;
			}
		};

		const auto base = reinterpret_cast<c_base_combat_weapon*>(entity);

		if (!is_nade_projectile(base->get_class_id()))
			return;

		nade_info current;
		current.origin = base->get_render_origin();
		current.type = base->get_class_id();
		info.nades.push_back(current);
	});

	const auto local = c_cs_player::get_local_player();

	if (!local)
		return;

	info.shoot_position = local->get_shoot_position();
	info.is_scoped = local->is_scoped();
}

void c_esp::draw_players()
{
	if (info.players.empty())
		return;

	for (auto& player : info.players)
		draw_player(player);
}

void c_esp::draw_nade(nade_info& nade)
{
	
	if (!config.esp.nade_esp)
		return;

	c_vector2d origin_screen;

	if (!renderer->screen_transform(nade.origin, origin_screen, info.world_to_screen_matrix))
		return;

	static const auto get_nade_string = [](int32_t definition) {
		switch (definition)
		{
		case cbasecsgrenadeprojectile:
			return std::string(_("GRENADE"));
		case cdecoyprojectile:
			return std::string(_("DECOY"));
		case csmokegrenadeprojectile:
			return std::string(_("SMOKE"));
		case cmolotovprojectile:
			return std::string(_("FIRE-BOMB"));
		default:
			return std::string();
		}
	};

	c_vector3d forward;
	math::angle_vectors(engine_client()->get_view_angles(), forward);
	const auto transform = math::rotation_matrix(forward);
	renderer->ball(nade.origin, 3.f, transform, config.esp.nade_color, info.world_to_screen_matrix);
	renderer->text(origin_screen - c_vector2d(0, 18), get_nade_string(nade.type).c_str(), config.esp.nade_color, fnv1a("pro13"), esp_flags);
	
}

void c_esp::draw_debug()
{

}

void c_esp::draw_nades()
{
	for (auto& nade : info.nades)
		draw_nade(nade);
}

void c_esp::draw_scope()
{
	if (!config.misc.no_scope || !info.is_scoped)
		return;

	const auto center = renderer->get_center();
	renderer->line({ center.x, 0 }, { center.x, 2.f * center.y }, c_color(0, 0, 0));
	renderer->line({ 0, center.y }, { 2.f * center.x, center.y }, c_color(0, 0, 0));

}

void c_esp::draw_impact(c_vector3d start, c_vector3d end, c_color color)
{
	beam_info info;

	info.type = 0;
	info.model_name = _("sprites/purplelaser1.vmt");
	info.model_index = -1;
	info.halo_scale = 0.f;
	info.life = 3.f;
	info.width = config.esp.local_impact_width;
	info.end_width = config.esp.local_impact_width;
	info.fade_length = 0.f;
	info.amplitude = 2.f;
	info.brightness = 255.f;
	info.speed = .2f;
	info.start_frame = 0;
	info.frame_rate = 0.f;
	info.red = float(color.red);
	info.green = float(color.green);
	info.blue = float(color.blue);
	info.segments = 2;
	info.renderable = true;
	info.flags = 0;
	info.start = start;
	info.end = end;

	auto beam = view_render_beams->create_beam_points(info);

	if (beam)
		view_render_beams->draw_beam(beam);
}

void c_esp::esp_name(player_info& player, const c_vector2d& from, const float width, const c_color& color)
{
	renderer->text(from + c_vector2d(width / 2.f, -9.f), player.name, color, fnv1a("pro13"), esp_flags);
}

void c_esp::esp_box(const c_vector2d& from, const float width, const float height, const c_color& color)
{
	renderer->rect(from - c_vector2d(1, 1), c_vector2d(width + 2, height + 2), c_color::shadow(color.alpha));
	renderer->rect(from + c_vector2d(1, 1), c_vector2d(width - 2, height - 2), c_color::shadow(color.alpha));
	renderer->rect(from, c_vector2d(width, height), color);
}

void c_esp::draw_player_flags(player_info& player, const c_vector2d& from, const float width, const float height, const c_color& color)
{
#ifdef PRINT_DEBUG
	printf("draw_player_flags \n");
#endif
 
	//Do all player flags inside of this
	int _x = from.x + width + 5, _y = from.y + 2;

	auto draw_flag = [&](c_color color, const char * text, ...) -> void
	{
		renderer->text(c_vector2d(_x, _y), text, color, fnv1a("pro13"), c_font::centered_y | c_font::font_flags::drop_shadow);
		_y += 8;
	};

	




	std::string text;

	if (player.b_helmet) text += "H";
	if (player.b_kevlar) text += "K";

	if (player.b_kevlar || player.b_helmet)
	{
		if (config.esp.enemy_flags.kevlar)
			draw_flag(c_color(config.esp.armor_color.red, config.esp.armor_color.green, config.esp.armor_color.blue, 255 * esp_alpha_fade[player.index]), text.c_str());
	}


	



	if (player.b_scoped && config.esp.enemy_flags.zoom)
		draw_flag(c_color(50, 50, 255, 255 * esp_alpha_fade[player.index]), "ZOOM");

	if (player.reload && player.layers[1].cycle < 0.99f && config.esp.enemy_flags.reload)
		draw_flag(c_color(255, 50, 50, 255 * esp_alpha_fade[player.index]), "RELOAD");

	if (player.b_defusing && config.esp.enemy_flags.defuse)
		draw_flag(c_color(255, 50, 50, 255 * esp_alpha_fade[player.index]), "DEFUSE");
	
	if (config.esp.enemy_flags.resolver)
	{
		//static const auto resolver_display = __("R MODE [%i]");
		//_rt(r_mode, resolver_display);

		//char r_msg[255];
		//sprintf_s(r_msg, r_mode, player.resolver_mode);

		static const auto shot_display = __("MISSED: %i");
		_rt(s_dis, shot_display);

		char shot_msg[255];
		sprintf_s(shot_msg, s_dis, player.shots_missed);

		draw_flag(c_color(255, 50, 50, 255 * esp_alpha_fade[player.index]), shot_msg);
		//draw_flag(c_color(64, 190, 59, 255 * esp_alpha_fade[player.index]), r_msg);
	}

	

}



void c_esp::esp_health(player_info& player, const c_vector2d& from, const float height, const c_color& color, bool should_devide)
{
#ifdef PRINT_DEBUG
	printf("esp_health \n");
#endif
	// grey background
	//renderer->rect_filled_linear_gradient(c_vector2d(from.x - 8, from.y), c_vector2d(4, height), c_color(20, 20, 20, 200), c_color(80, 80, 80, 200));
	float addshit = 1;

	const auto health_removed = 100 - player.health;
	const auto health_scaled = static_cast<float>(health_removed) / 100.f * (height + 2);
	auto c = config.esp.health_color;
	int Red = config.esp.health_based ? 255 - (player.health * 2.55) : c.red;
	int Green = config.esp.health_based ? player.health * 2.55 : c.green;

	renderer->rect_filled(c_vector2d(from.x - 6, from.y - 1), c_vector2d(3, height + 2), c_color(0, 0, 0, 100 * esp_alpha_fade[player.index]));


	renderer->rect_filled(c_vector2d(from.x - 6, from.y - 1 + health_scaled), c_vector2d(3, height + 2 - health_scaled), c_color(Red, Green, config.esp.health_based ? 0 : c.blue, (255 * esp_alpha_fade[player.index])));


	renderer->rect(c_vector2d(from.x - 6, from.y - 1), c_vector2d(3, height + 2), c_color(0, 0, 0, 255 * esp_alpha_fade[player.index]));
}


void c_esp::esp_armor(player_info& player, const c_vector2d& from, const float height, const c_color& color, bool should_devide)
{
#ifdef PRINT_DEBUG
	printf("esp_armor \n");
#endif
	// grey background
	//renderer->rect_filled_linear_gradient(c_vector2d(from.x - 8, from.y), c_vector2d(4, height), c_color(20, 20, 20, 200), c_color(80, 80, 80, 200));
	float addshit = config.esp.enemy.health ? 12 : 6;
	const auto health_removed = 100 - player.player->get_armor();
	const auto health_scaled = static_cast<float>(health_removed) / 100.f * (height + 2);
	auto c = config.esp.armor_color;
	int Red =  c.red;
	int Green = c.green;
	renderer->rect_filled(c_vector2d(from.x - addshit, from.y - 1), c_vector2d(3, height + 2), c_color(0, 0, 0, 100 * esp_alpha_fade[player.index]));

	
	renderer->rect_filled(c_vector2d(from.x - addshit, from.y - 1 + health_scaled), c_vector2d(3, height + 2 - health_scaled), c_color(Red, Green, c.blue, (255 * esp_alpha_fade[player.index])));

	
	renderer->rect(c_vector2d(from.x - addshit, from.y - 1), c_vector2d(3, height + 2), c_color(0, 0, 0, 255 * esp_alpha_fade[player.index]));

}


void c_esp::esp_ammo(player_info& player, const c_vector2d& from, const float width, const float height, const c_color& color)
{
	if ((player.ammo <= 0 || player.ammo_max <= 1) && !player.reload)
		return;

	auto ammo = static_cast<float>(player.ammo) / static_cast<float>(player.ammo_max) * (width+2);

	if (player.reload && player.layers[1].cycle > 0.f)
		ammo = player.layers[1].cycle * width;

	renderer->rect_filled(c_vector2d(from.x - 1, from.y + height + 3), c_vector2d(width + 2, 3), c_color(0, 0, 0, 100 * esp_alpha_fade[player.index]));
	static float usin = 0.f;
	
		renderer->rect_filled(c_vector2d(from.x - 1, from.y + height + 3), c_vector2d(ammo, 3), color);
		usin = ammo;
	

	

	// black outline
	renderer->rect(c_vector2d(from.x - 1, from.y + height + 3), c_vector2d(width+2, 3), c_color(0, 0, 0, 255 * esp_alpha_fade[player.index]));

	//if (player.ammo < player.ammo_max || player.reload) {
	//	renderer->text(c_vector2d(from.x + usin, from.y + height + 3), player.reload ? "RELOAD" : std::to_string(player.ammo).c_str(), c_color(255, 255, 255, 255), fnv1a("pro13"), esp_flags);
	//}
}


void c_esp::esp_radar(std::optional<c_vector3d> position, const c_color& color)
{
	if (!position.has_value())
		return;

	const auto local = c_cs_player::get_local_player();

	if (!local)
		return;
	
	const auto view = engine_client()->get_view_angles();
	const auto angle_to = math::calc_angle(info.shoot_position, position.value());
	auto target_angle = angle_to - view;
	math::normalize(target_angle);

	const auto angle = target_angle.y;
	const auto height = 500.f;

	const auto a = renderer->get_center() - c_vector2d(
		(height - 20) * sin(deg2rad(angle + 2)),
		(height - 20) * cos(deg2rad(angle + 2))
	);

	const auto b = renderer->get_center() - c_vector2d(
		height * sin(deg2rad(angle)),
		height * cos(deg2rad(angle))
	);

	const auto c = renderer->get_center() - c_vector2d(
		(height - 20) * sin(deg2rad(angle - 2)),
		(height - 20) * cos(deg2rad(angle - 2))
	);

	renderer->triangle_filled(a, b, c, c_color(color.red, color.green, color.blue, std::clamp(color.alpha,0, 100)));
	renderer->triangle(a, b, c, c_color(color.red, color.green, color.blue, color.alpha));
}

void c_esp::esp_skeleton(player_info& player, const c_color& color, bool history)
{
	c_vector2d bone, bone_parent;

	const auto entity = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(player.index));
	
	if (!entity)
		return;

	const auto model = entity->get_model();

	if (!model)
		return;

	const auto studio_model = model_info_client()->get_studio_model(model);

	if (!studio_model)
		return;

	if (history)
	{
		matrix3x4 interpolated[128] = {};

		if (c_chams::get_simple_backtrack_matrix(player.player, interpolated))
		{
			for (auto i = 0; i < studio_model->numbones; i++)
			{
				const auto p_bone = studio_model->get_bone(i);

				if (!p_bone || p_bone->parent == -1)
					continue;

				if (!(p_bone->flags & bone_used_by_hitbox))
					continue;

				if (!renderer->screen_transform(c_vector3d(interpolated[i][0][3], interpolated[i][1][3], interpolated[i][2][3]), bone, info.world_to_screen_matrix) 
					|| !renderer->screen_transform(c_vector3d(interpolated[p_bone->parent][0][3], interpolated[p_bone->parent][1][3], interpolated[p_bone->parent][2][3]), bone_parent, info.world_to_screen_matrix))
					continue;

				renderer->line(bone, bone_parent, color);
			}
		}

		/*DRAW STANDARD IF NOT HISTROY! :) */

		if (!player.good_bones)
			return;

		for (auto i = 0; i < studio_model->numbones; i++)
		{
			const auto p_bone = studio_model->get_bone(i);

			if (!p_bone || p_bone->parent == -1)
				continue;

			if (!(p_bone->flags & bone_used_by_hitbox))
				continue;

			if (!renderer->screen_transform(c_vector3d(player.bones[i][0][3], player.bones[i][1][3], player.bones[i][2][3]), bone, info.world_to_screen_matrix) || !renderer->screen_transform(c_vector3d(player.bones[p_bone->parent][0][3], player.bones[p_bone->parent][1][3], player.bones[p_bone->parent][2][3]), bone_parent, info.world_to_screen_matrix))
				continue;

			renderer->line(bone, bone_parent, color);
		}
	}
	else
	{

		if (!player.good_bones)
			return;

		for (auto i = 0; i < studio_model->numbones; i++)
		{
			const auto p_bone = studio_model->get_bone(i);

			if (!p_bone || p_bone->parent == -1)
				continue;

			if (!(p_bone->flags & bone_used_by_hitbox))
				continue;

			if (!renderer->screen_transform(c_vector3d(player.bones[i][0][3], player.bones[i][1][3], player.bones[i][2][3]), bone, info.world_to_screen_matrix) || !renderer->screen_transform(c_vector3d(player.bones[p_bone->parent][0][3], player.bones[p_bone->parent][1][3], player.bones[p_bone->parent][2][3]), bone_parent, info.world_to_screen_matrix))
				continue;

			renderer->line(bone, bone_parent, color);
		}

	}
}



void c_esp::esp_weapon(player_info& player, const c_vector2d& from, const float width, const float height, const c_color& color, bool draw_ammo)
{

	if (!player.pWeapon)
		return;

	bool IsAmmoDrawing = (player.ammo <= 0 || player.ammo_max <= 1) && !player.reload;
	float additive = 9;
	if (IsAmmoDrawing) additive += 9;
	if (IsAmmoDrawing && config.esp.wep_icons) additive += 5;

		if(config.esp.wep_icons)
			renderer->text(c_vector2d(from.x + (width / 2), from.y + height + additive), player.pWeapon->GetGunIcon(), color, fnv1a("weapons"), esp_flags);
		else
			renderer->text(c_vector2d(from.x + (width / 2), from.y + height + additive), player.pWeapon->get_weapon_name(), color, fnv1a("pro13"), esp_flags);

	
	
}

void c_esp::draw_player(player_info& player)
{
	if (!player.head_position.has_value())
		return;

	// crashfix, do not remove 
	// ^^-- nice you dont fucking say ocorn! fucking genius --^^ -- loki was here
	if (!engine_client()->is_ingame() || !engine_client()->is_connected())
		return;

	if (!player.is_enemy)
		return;

	int idx = player.index;

	float in = (1.f / 0.2f) * global_vars_base->frametime;
	float out = (1.f / 2.f) * global_vars_base->frametime;

	if (!player.dormant)
	{
		if (esp_alpha_fade[idx] < 1.f)
			esp_alpha_fade[idx] += in;
	}
	else
	{
		if (esp_alpha_fade[idx] > 0.f)
			esp_alpha_fade[idx] -= out;
	}

	esp_alpha_fade[idx] = (esp_alpha_fade[idx] > 1.f ? 1.f : esp_alpha_fade[idx] < 0.f ? 0.f : esp_alpha_fade[idx]);

	const auto& esp = config.esp.enemy;

	const auto& esp_main = config.esp;

	c_vector2d origin_screen, top_screen;

	const auto collision_box_interpolated = collision_box_top - collision_box_mod * player.duck_amount;

	const auto view_height = player.head_position.value().z - player.origin.z;

	auto top = player.origin + c_vector3d(0.f, 0.f, collision_box_interpolated);

	if (view_height == 46.f && !player.is_on_ground)
		top = player.head_position.value_or(c_vector3d()) + c_vector3d(0.f, 0.f, 9.f);

	if (!renderer->screen_transform(player.origin, origin_screen, info.world_to_screen_matrix) || !renderer->screen_transform(top, top_screen, info.world_to_screen_matrix))
	{
		if (esp.radar && 255 * esp_alpha_fade[idx] == 255)
			esp_radar(player.head_position, c_color(esp_main.radar_color.red, esp_main.radar_color.green, esp_main.radar_color.blue, 255 * esp_alpha_fade[idx]));

		return;
	}

	const auto height = origin_screen.y - top_screen.y + 6;
	const auto width = height / 1.9f;
	const auto from = c_vector2d(top_screen.x - width / 2.f, top_screen.y);

	auto scale_and_draw = [](c_cs_player* entity, c_cs_player::hitbox hitbox, float point_scale, matrix3x4* bones_t)
	{
		if (!entity)
			return;

		c_vector3d scaled[7] = { };
		c_vector2d scaled_screenspace[7] = { };
		auto hitbox_pos = entity->get_hitbox_position(hitbox, bones_t);

		if (!hitbox_pos.has_value())
			return;

		std::fill_n(scaled, 7, hitbox_pos.value());

		scaled[1][2] += point_scale;
		scaled[2][2] -= point_scale;
		scaled[3][1] -= point_scale;
		scaled[4][1] += point_scale;

		int max_num = hitbox == c_cs_player::hitbox::head ? 3 : 5;

		for (int i = 0; i < max_num; i++)
		{
			if (!renderer->screen_transform(scaled[i], scaled_screenspace[i], info.world_to_screen_matrix))
				return;

			renderer->rect(c_vector2d((int)scaled_screenspace[i].x - 2, (int)scaled_screenspace[i].y - 2), c_vector2d(4, 4), c_color(255, 0, 0, 255 * esp_alpha_fade[entity->index()]));
		}
	};

	if (esp.radar && config.misc.indicators && !renderer->is_on_screen(player.head_position.value_or(player.origin), width, info.world_to_screen_matrix))
		esp_radar(player.head_position, c_color(esp_main.radar_color.red, esp_main.radar_color.green, esp_main.radar_color.blue, esp_main.radar_color.alpha * esp_alpha_fade[idx]));

	if (esp.skeleton)
		esp_skeleton(player, c_color(esp_main.skeleton_color.red, esp_main.skeleton_color.green, esp_main.skeleton_color.blue, esp_main.skeleton_color.alpha * esp_alpha_fade[idx]), esp.history_skeleton);

	if (esp.box)
		esp_box(from, width, height, c_color(esp_main.box_color.red, esp_main.box_color.green, esp_main.box_color.blue, esp_main.box_color.alpha * esp_alpha_fade[idx]));

	if (esp.health)
		esp_health(player, from, height, c_color(esp_main.color.red, esp_main.color.green, esp_main.color.blue, esp_main.health_color.alpha * esp_alpha_fade[idx]), false);

	if(config.esp.armor)
		esp_armor(player, from, height, c_color(config.esp.armor_color.red, config.esp.armor_color.green, config.esp.armor_color.blue, config.esp.armor_color.alpha * esp_alpha_fade[idx]), false);

	if (esp.ammo)
		esp_ammo(player, from, width, height, c_color(esp_main.ammo_color.red, esp_main.ammo_color.green, esp_main.ammo_color.blue, esp_main.ammo_color.alpha * esp_alpha_fade[idx]));

	
	if (esp.name)
		esp_name(player, from, width, c_color(config.esp.name_color.red, config.esp.name_color.green, config.esp.name_color.blue, config.esp.name_color.alpha * esp_alpha_fade[idx]));

	if (esp.weapon)
		esp_weapon(player, from, width, height, c_color(esp_main.weapon_color.red, esp_main.weapon_color.green, esp_main.weapon_color.blue, esp_main.weapon_color.alpha * esp_alpha_fade[idx]), esp.ammo);

	if (esp.flags)
		draw_player_flags(player, from, width, height, c_color(esp_main.color.red, esp_main.color.green, esp_main.color.blue, 255 * esp_alpha_fade[idx]));

	pen_crosshair();
	//draw_recoilcrosshair();
	if (config.esp.multipoint)
	{
		c_cs_player::hitbox hitboxes[] = { c_cs_player::hitbox::head, c_cs_player::hitbox::pelvis, c_cs_player::hitbox::chest, c_cs_player::hitbox::thorax, c_cs_player::hitbox::left_thigh, c_cs_player::hitbox::right_thigh, c_cs_player::hitbox::left_foot, c_cs_player::hitbox::right_foot , c_cs_player::hitbox::right_upper_arm, c_cs_player::hitbox::left_upper_arm };

		for (int i = 0; i < _ARRAYSIZE(hitboxes); i++)
		{
			float pointscale = i == 0 ? 1.5f : 4.f;
			scale_and_draw(player.player, hitboxes[i], pointscale, player.bones);
		}
	}
}



struct damage_infos {
	c_cs_player* player{};
	char name[16]{};
	int damage{};
	int last{};
	int shots{};
	int miss{};
	bool killed{};
};
std::vector<damage_infos> dmg_info;

void c_esp::clear_dmg() {
	dmg_info.clear();
}
void c_esp::draw_recoilcrosshair() {
	auto local = c_cs_player::get_local_player();
	if (!local || local->get_health() <= 0)return;

	c_vector3d fowardVec;
	auto viewangles = engine_client()->get_view_angles();
	static const auto precoil_scale = cvar()->find_var(_("weapon_recoil_scale"));
	const auto recoil = precoil_scale->get_float();
	
	viewangles += local->get_punch_angle() * recoil;
	math::angle_vectors(viewangles, fowardVec);
	fowardVec *= 10000;//cuz i like even numbers

	c_vector3d start = local->get_shoot_position();
	c_vector3d end = start + fowardVec, out3d;
	c_vector2d out;

	if (!renderer->world2screen(end, out3d, engine_client()->get_matrix()))return;

	out = c_vector2d(out3d.x, out3d.y);
	
	renderer->line(out - c_vector2d(3, 0), out + c_vector2d(4, 0), c_color(255, 255, 255));
	renderer->line(out - c_vector2d(0, 3), out + c_vector2d(0, 4), c_color(255, 255, 255));
}
void c_esp::draw_dmg() {
	if (!config.misc.on_screenplayers) return;
	c_vector2d position = { (float)config.misc.on_screen_x,(float)config.misc.on_screen_y+45 };
	auto local = c_cs_player::get_local_player();
	if (!local){ clear_dmg(); return; }

	int x_off = 50;
	int offset = 0;
	auto draw_text = [](const char* text, c_vector2d pos, int &off, c_color Color, int off2) {
		renderer->text(c_vector2d(pos.x + off2,  pos.y + off), text, Color, fnv1a("pro13"), c_font::font_flags::drop_shadow | c_font::font_flags::centered_x);
	};
	for (const auto &it : dmg_info) {
		int off = 0;
		draw_text(it.name, position, offset, it.killed ? c_color(255, 100, 100) : c_color(255, 255, 255), off);
		off += x_off;

		std::string damage = "Damage: " + std::to_string(it.damage);
		draw_text(damage.c_str(), position + c_vector2d(100, 0), offset, it.damage >= 100 ? c_color(255,100,100) : c_color(255, 255, 255), off);
		off += x_off;

		


		char * hitgroup = "generic";
		switch (it.last)
		{
		case hitgroup_generic: hitgroup = "Generic";  break;
		case hitgroup_head: hitgroup = "Head";  break;
		case hitgroup_chest: hitgroup = "Chest";  break;
		case hitgroup_stomach: hitgroup = "Stomach";  break;
		case hitgroup_leftarm: hitgroup = "Left Arm";  break;
		case hitgroup_rightarm: hitgroup = "Right Arm";  break;
		case hitgroup_leftleg: hitgroup = "Left Leg";  break;
		case hitgroup_rightleg: hitgroup = "Right Leg";  break;
		case hitgroup_gear: hitgroup = "Gear";  break;
		default: hitgroup = "Generic"; break;
		}

		draw_text(hitgroup, position + c_vector2d(200, 0), offset, c_color(255, 255, 255), off);
		off += x_off;


		offset += 13;
	}
	

}

void c_esp::adddmg(c_game_event* event) {
	auto local = c_cs_player::get_local_player();


	const auto attacker = client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("attacker"))));
	const auto target = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("userid")))));

	if (!local || !local->is_alive())
		return;
	

	if (attacker && target && attacker == local && target->is_enemy())
	{
		bool exists = false;
		int point = 0;
		int i = 0;
		for (const auto &it : dmg_info) {
			if (exists) continue;
			if (it.player == target) {
				exists = true;
				point = i;
			}
			i++;
		}
		
		if (exists) {
			dmg_info.at(point).damage = dmg_info.at(point).damage + event->get_int(_("dmg_health"));
			dmg_info.at(point).last = event->get_int(_("hitgroup"));
		}
		else {

			damage_infos toadd;
			toadd.player = target;
			memcpy(toadd.name, target->get_info().name, 15);
			toadd.name[15] = '\0';
			toadd.damage = event->get_int(_("dmg_health"));
			toadd.last = event->get_int(_("hitgroup"));
			dmg_info.push_back(toadd);
		}
	}
}

void c_esp::AddShot(c_cs_player* target, int type) {
	bool exists = false;
	int point = 0;
	int i = 0;
	for (const auto &it : dmg_info) {
		if (exists) continue;
		if (it.player == target) {
			exists = true;
			point = i;
		}
		i++;
	}
	if (exists) {
		switch (type) {
		case 0: // shot
			dmg_info.at(point).shots = dmg_info.at(point).shots + 1;
			break;
		case 1: // miss
			dmg_info.at(point).miss = dmg_info.at(point).miss + 1;
			break;
		case 2: // kill
			dmg_info.at(point).killed = true;
		}
	}
	else {
		damage_infos toadd;
		toadd.player = target;
		memcpy(toadd.name, target->get_info().name, 15);
		toadd.name[15] = '\0';
		toadd.damage = 0;
		toadd.last = 0;
		dmg_info.at(point).shots = type == 0 ? 1 : 0;
		dmg_info.at(point).miss = type == 1 ? 1 : 0;
		dmg_info.at(point).miss = type == 2;
		dmg_info.push_back(toadd);
	}
}