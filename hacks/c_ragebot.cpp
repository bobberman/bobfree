
#include "c_ragebot.h"
#include "c_aimhelper.h"
#include "c_trace_system.h"
#include "../utils/math.h"
#include "../sdk/c_weapon_system.h"
#include "../sdk/c_debug_overlay.h"
#include "c_prediction_system.h"
#include "c_antiaim.h"
#include "c_resolver.h"
#include "../menu/c_menu.h"
#include "../hacks/c_hitmarker.h"

void c_ragebot::aim(c_cs_player* local, c_user_cmd* cmd, bool& send_packet)
{
	last_pitch = std::nullopt;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));
	if (!weapon)
		return;

	const auto wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition());
	if (!wpn_info)
		return;

	if (!local->can_shoot(cmd, global_vars_base->curtime))
		return;

	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (!weapon_cfg.has_value())
		return;

	std::vector<aim_info> hitpoints = {};

	client_entity_list()->for_each_player([&](c_cs_player* player) -> void
	{
		if (!player->is_enemy() || !player->is_alive() || player->get_gun_game_immunity())
			return;

		const auto latest = animation_system->get_latest_animation(player);

		if (!latest.has_value())
			return;

		const auto rtt = 2.f * net_channel->get_latency(flow_outgoing);
		const auto breaking_lagcomp = latest.value()->lag && latest.value()->lag <= 17 && is_breaking_lagcomp(latest.value());
		const auto can_delay_shot = (latest.value()->lag > time_to_ticks(rtt) + global_vars_base->interval_per_tick);
		const auto delay_shot = (time_to_ticks(rtt) + time_to_ticks(global_vars_base->curtime - latest.value()->sim_time) + global_vars_base->interval_per_tick >= latest.value()->lag);
		const auto oldest = animation_system->get_oldest_animation(player);
		const auto firing = animation_system->get_latest_firing_animation(player); //firing
		const auto upPitch = animation_system->get_latest_upPitch_animation(player); //uppitch
		const auto sideways = animation_system->get_latest_sideways_animation(player); //sideways
		const auto uncrouched = animation_system->get_uncrouched_animation(player);

		if (breaking_lagcomp && delay_shot && can_delay_shot)
			return;

		if (breaking_lagcomp && !can_delay_shot)
			return;

		/*
		priority order://highest chance of hitting
		firing
		uppitch
		sideways
		uncrouched
		latest
		oldest
		
		*/
		/*
		we could add if the target is sideways to us
		we could add something to check if its the highest damage potential (kill potential, prio a kill over doing dmg)
		scan more than first and last

		*/


		std::optional<aim_info> aimbot_info;

		if (firing.has_value()) {
			const auto alternative = scan_record(local, firing.value());//shooting
			if (alternative.has_value()) {
				if (!aimbot_info.has_value() || (alternative.value().damage > aimbot_info.value().damage))
					aimbot_info = alternative;
			}
		}
		else if (sideways.has_value()) {
			const auto alternative = scan_record(local, sideways.value());//sideways fuck desync
			if (alternative.has_value()) {
				if (!aimbot_info.has_value() || (alternative.value().damage > aimbot_info.value().damage))
					aimbot_info = alternative;
			}
		}
		else if (upPitch.has_value()) {
			const auto alternative = scan_record(local, upPitch.value());//lookingup
			if (alternative.has_value()) {
				if (!aimbot_info.has_value() || (alternative.value().damage > aimbot_info.value().damage))
					aimbot_info = alternative;
			}
		}
		
		else if (!aimbot_info.has_value() && uncrouched.has_value()){//use this only if we have nothing better lol, i suppose this is against fakeduck?
			const auto alternative = scan_record(local, uncrouched.value());//uncroched cuz we are gay or something?
			if (alternative.has_value() &&
				(!aimbot_info.has_value() || alternative.value().damage > aimbot_info.value().damage))
				aimbot_info = alternative;
		}
		else if(!aimbot_info.has_value()){//we are running out of ideas, try some normal stuff
			const auto alternative = scan_record(local, latest.value());//latest
			if (alternative.has_value() && 
				(!aimbot_info.has_value() || aimbot_info.value().damage < alternative.value().damage))
				aimbot_info = alternative;
		}
		else if (!aimbot_info.has_value() && oldest.has_value() && latest.value() != oldest.value() /*&& oldest.value()->velocity.length2d() >= .1f*/) {//no need for velocity check he could have changed angles
			const auto alternative = scan_record(local, oldest.value());
			// is there no other record?
			if ((alternative.has_value() && !aimbot_info.has_value()) ||
				/*(aimbot_info.has_value() && aimbot_info.value().animation->velocity.length2d() < .1f) ||*///fucking useless
				(alternative.has_value() && aimbot_info.has_value() && alternative.value().damage > aimbot_info.value().damage))
				aimbot_info = alternative;
		}
			
		if (aimbot_info.has_value())
			hitpoints.push_back(aimbot_info.value());
	});

	aim_info best_match = { c_vector3d(), -FLT_MAX, nullptr, false, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };

	// find best target spot of all valid spots.
	for (auto& hitpoint : hitpoints)
		if (hitpoint.damage > best_match.damage)
			best_match = hitpoint;

	// stop if no target found.
	if (best_match.damage < 0.f)
		return;

	// run autostop.
	if (cmd->buttons & ~c_user_cmd::jump)
	{
		autostop(local, cmd);
	}
	if (config.rage.fakelag_settings.fake_lag_on_peek_delay && antiaim->is_on_peek)
	{
		const auto entity_weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(best_match.animation->player->get_current_weapon_handle()));

		auto should_return = true;
		auto on_ground = best_match.animation->player->is_on_ground();

		c_base_animating::animation_layers layers = *best_match.animation->player->get_animation_layers();

		if (best_match.animation->player->get_health() <= weapon_cfg.value().body_aim_health)
			should_return = false;

		if (entity_weapon && entity_weapon->get_current_clip() <= 2)
			should_return = false;

		if (!on_ground)
			should_return = false;

		if (best_match.animation->player->get_sequence_activity(layers[1].sequence) == act_csgo_reload && layers[1].cycle < 0.99f)
			should_return = false;

		if (entity_weapon && entity_weapon->get_item_definition() == weapon_knife || entity_weapon->get_item_definition() == weapon_knife_t || entity_weapon->get_item_definition() == weapon_taser || entity_weapon->get_item_definition() == weapon_c4)
			should_return = false;

		if (should_return && !best_match.animation->didshot && !best_match.animation->upPitch && !best_match.animation->sideways)
			return;
	}

	// scope the weapon.
	if ((wpn_info->get_weapon_id() == weapon_g3sg1 || wpn_info->get_weapon_id() == weapon_scar20 || wpn_info->get_weapon_id() == weapon_ssg08 || wpn_info->get_weapon_id() == weapon_awp 
		|| wpn_info->get_weapon_id() == weapon_sg556 || wpn_info->get_weapon_id() == weapon_aug) && weapon->get_zoom_level() == 0)
		cmd->buttons |= c_user_cmd::flags::attack2;
	
	// calculate angle.
	const auto angle = math::calc_angle(local->get_shoot_position(), best_match.position);

	// store pitch for eye correction.
	last_pitch = angle.x;

	// optimize multipoint and select final aimpoint.
	//c_aimhelper::optimize_multipoint(best_match);//nice to call this but its not doing anything, not changing the targetangle

	//debug_overlay()->add_line_overlay(local->get_shoot_position(), best_match.position, 255, 0, 0, true, 0.1);

	// auto doesmatch = best_match.animation->player->get_health() <= weapon_cfg.value().hp_health_override;

	if (c_aimhelper::is_hitbox_a_body(best_match.hitbox))
	{
		if (!c_aimhelper::can_hit(local, best_match.animation, best_match.position, weapon_cfg.value().hitchance_body / 100.f, best_match.hitbox))
			return;
	}
	else if (c_aimhelper::is_hitbox_a_body(best_match.hitbox) && best_match.animation->player->get_health() <= weapon_cfg.value().hp_health_override)
	{
		if (!c_aimhelper::can_hit(local, best_match.animation, best_match.position, weapon_cfg.value().hp_body_hitchance / 100.f, best_match.hitbox))
			return;
	}
	else if (!(c_aimhelper::is_hitbox_a_body(best_match.hitbox)) && best_match.animation->player->get_health() <= weapon_cfg.value().hp_health_override)
	{
		if (!c_aimhelper::can_hit(local, best_match.animation, best_match.position, weapon_cfg.value().hp_head_hitchance / 100.f, best_match.hitbox))
			return;
	}
	else
		if (!c_aimhelper::can_hit(local, best_match.animation, best_match.position, weapon_cfg.value().hitchance_head / 100.f, best_match.hitbox))
			return;

	// store shot info for resolver.
	if (!best_match.alt_attack)
	{
		resolver::shot shot{};
		shot.damage = best_match.damage;
		shot.start = local->get_shoot_position();
		shot.end = best_match.position;
		shot.hitgroup = best_match.hitgroup;
		shot.hitbox = best_match.hitbox;
		shot.time = global_vars_base->curtime;
		shot.record = *best_match.animation;
		shot.manual = false;
		shot.tickcount = cmd->tick_count - time_to_ticks(best_match.animation->sim_time) + time_to_ticks(calculate_lerp());
		shot.sideways = best_match.animation->sideways;
		shot.uppitch = best_match.animation->upPitch;
		shot.shotting = best_match.animation->didshot;
		shot.index = best_match.animation->index;



		char msg[255];
		static const auto hit_msg = __("Fired at with BT %d. Type: %s Hitbox: %s");
		_rt(hit, hit_msg);
		char type[255]; char phitbox[255];
		if (shot.shotting)
			sprintf(type, "Shooting");
		else if (shot.sideways)
			sprintf(type, "SideW");
		else if (shot.uppitch)
			sprintf(type, "UP");
		else
			sprintf(type, "Nothing");

		switch (shot.hitbox) {
		case c_cs_player::hitbox::head:
			sprintf(phitbox, "Head");
			break;
		case c_cs_player::hitbox::pelvis:
			sprintf(phitbox, "Pelvis");
			break;
		default:
			sprintf(phitbox, "Unk");
			break;
		}

		sprintf_s(msg, hit, shot.tickcount, type, phitbox);

		logging->info(msg);

		switch (config.rage.resolver) {
		case 1:c_resolver::register_shot(std::move(shot));
			break;
		case 2:c_resolver_beta::register_shot(std::move(shot));
			break;
		}

		
			
	}

	// set correct information to user_cmd.
	cmd->viewangles = angle;
	cmd->tick_count = time_to_ticks(best_match.animation->sim_time + calculate_lerp()) ;//+ time_to_ticks()

	if (weapon_cfg.value().auto_shoot && GetAsyncKeyState(config.rage.fake_duck))
	{
		if (local->get_duck_amount() == 0.f)
		{
			cmd->buttons |= best_match.alt_attack ? c_user_cmd::attack2 : c_user_cmd::attack;

		}
	}
	else if (weapon_cfg.value().auto_shoot && cmd->buttons & c_user_cmd::duck)
	{
	cmd->buttons |= best_match.alt_attack ? c_user_cmd::attack2 : c_user_cmd::attack;
	}
	else
	{
		cmd->buttons |= best_match.alt_attack ? c_user_cmd::attack2 : c_user_cmd::attack;
	}
}

void c_ragebot::autostop(c_cs_player* local, c_user_cmd* cmd)
{
	if (cmd->buttons & c_user_cmd::jump)
		return;

	 static const auto nospread = cvar()->find_var(_("weapon_accuracy_nospread"));

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (nospread->get_int()   ||  !local->is_on_ground() ||
		(weapon && weapon->get_item_definition() == weapon_taser) && local->is_on_ground())
		return;

	const auto wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!wpn_info)
		return;

	auto& info = get_autostop_info(cmd);

	if (info.call_time == global_vars_base->curtime)
	{
		info.did_stop = true;
		return;
	}

	info.did_stop = false;
	info.call_time = global_vars_base->curtime;
	
	if (local->get_velocity().length2d() <= wpn_info->get_standing_accuracy(weapon))
		return;
	else
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;

		prediction_system->repredict(local, cmd);

		if (config.rage.slow_walk && GetAsyncKeyState(config.rage.slow_walk))
		{
			antiaim->is_slow_walking = true;
			info.did_stop = true;
			return;
		}
		else
			antiaim->is_slow_walking = false;

		if (local->get_velocity().length2d() <= wpn_info->get_standing_accuracy(weapon))
			return;
	}

	c_qangle dir;
	math::vector_angles(prediction_system->unpredicted_velocity, dir);
	const auto angles = engine_client()->get_view_angles();
	dir.y = angles.y - dir.y;

	c_vector3d move;
	math::angle_vectors(dir, move);

	if (prediction_system->unpredicted_velocity.length2d() > .1f)
		move *= -math::forward_bounds / std::max(std::abs(move.x), std::abs(move.y));

	cmd->forwardmove = move.x;
	cmd->sidemove = move.y;

	const auto backup = cmd->viewangles;
	cmd->viewangles = angles;
	prediction_system->repredict(local, cmd);
	cmd->viewangles = backup;

	if (local->get_velocity().length2d() > prediction_system->unpredicted_velocity.length2d())
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
	}

	prediction_system->repredict(local, cmd);
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record(c_cs_player* local, c_animation_system::animation* animation)
{
	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return std::nullopt;

	const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!info)
		return std::nullopt;

	const auto is_zeus = weapon->get_item_definition() == weapon_taser;
	const auto is_knife = !is_zeus && info->WeaponType == weapontype_knife;

	if (is_knife)
		return scan_record_knife(local, animation);

	return scan_record_aimbot(local, animation);
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record_aimbot(c_cs_player * local, c_animation_system::animation* animation, std::optional<c_vector3d> pos)
{
	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (!animation || !animation->player || !weapon_cfg.has_value())
		return std::nullopt;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return std::nullopt;

	const auto info = animation_system->get_animation_info(animation->player);

	if (!info)
		return std::nullopt;

	const auto cfg = weapon_cfg.value();

	/*TODO:
		89 1D ? ? ? ? 8B C3 + 2 + //g_netgraph xref: "vgui/white",
		0x131B8 = (int)(1.0f / m_Framerate) lets use the game :)
		FPS OPTIMIZATION MODE USING ABOVE
	*/
	const float client_frame_rate = (int)(1.0f / *(float*)(net_graph + 0x131B8));
	
	auto should_optimize = client_frame_rate <= cfg.optimize_fps;

	auto should_baim = false;

	const auto center = animation->player->get_hitbox_position(c_cs_player::hitbox::pelvis, animation->bones);

	const auto slow_walk = animation->anim_state.feet_yaw_rate >= 0.01 && animation->anim_state.feet_yaw_rate <= 0.8;

	const auto is_moving = animation->velocity.length2d() > 0.1f && animation->player->is_on_ground() && !slow_walk;

	if (center.has_value())
	{
		const auto center_wall = trace_system->wall_penetration(pos.value_or(local->get_shoot_position()),
			center.value(), animation);

		if (center_wall.has_value() && center_wall.value().hitbox == c_cs_player::hitbox::pelvis
			&& center_wall.value().damage - 10.f > animation->player->get_health())
			should_baim = true;
	}

	if (!should_baim)
	{
		const auto data = weapon_system->get_weapon_data(weapon->get_item_definition());

		if (!data)
			return std::nullopt;

		if (animation->player->get_health() <= cfg.body_aim_health)
			should_baim = true;

		if (cfg.smart_aim)
		{
			if (animation->player->get_health() <= 50)
				should_baim = true;

			if (animation->player->get_velocity().length2d() > 0.1 && !animation->player->is_on_ground())
				should_baim = true;

			if (slow_walk)
				should_baim = true;
		}

		if (cfg.body_aim_in_air)
			if (animation->player->get_velocity().length2d() > 0.1 && !animation->player->is_on_ground())
				should_baim = true;

		if (cfg.body_aim_lethal && animation->player->get_health() < data->iDamage)
			should_baim = true;

		if (cfg.body_aim_slow_walk)
			if (slow_walk)
				should_baim = true;

		if (GetAsyncKeyState(cfg.body_aim_key))
			should_baim = true;

		if (info->missed_due_to_resolver >= cfg.body_after_x_missed_resolver + 1)
			should_baim = true;

		if (info->missed_due_to_spread >= cfg.body_after_x_missed_spread + 1)
			should_baim = true;

		if (cfg.body_aim_if_not_on_shot && !animation->didshot)
			should_baim = true;
	}

	aim_info best_match = { c_vector3d(), -FLT_MAX, nullptr, false, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };

	const auto scan_box = [&](c_cs_player::hitbox hitbox)
	{
		auto box = animation->player->get_hitbox_position(hitbox, const_cast<matrix3x4*>(animation->bones));

		if (!box.has_value())
			return;

		auto scale_hitbox = 0.f;

		switch (hitbox)
		{
		case c_cs_player::hitbox::head:
			scale_hitbox = cfg.head_scale / 100.f;
			break;
		case c_cs_player::hitbox::neck:
			scale_hitbox = cfg.head_scale / 100.f;
			break;
		case c_cs_player::hitbox::upper_chest:
			scale_hitbox = cfg.chest_scale / 100.f;
			break;
		case c_cs_player::hitbox::chest:
			scale_hitbox = cfg.chest_scale / 100.f;
			break;
		case c_cs_player::hitbox::thorax:
			scale_hitbox = cfg.stomach_scale / 100.f;
			break;
		case c_cs_player::hitbox::pelvis:
			scale_hitbox = cfg.pelvis_scale / 100.f;
			break;
		case c_cs_player::hitbox::left_thigh:
			scale_hitbox = cfg.legs_scale / 100.f;
			break;
		case c_cs_player::hitbox::right_thigh:
			scale_hitbox = cfg.legs_scale / 100.f;
			break;
		case c_cs_player::hitbox::left_foot:
			scale_hitbox = cfg.feet_scale / 100.f;
			break;
		case c_cs_player::hitbox::right_foot:
			scale_hitbox = cfg.feet_scale / 100.f;
			break;
		}

		auto points = pos.has_value() ? std::vector<aim_info>() : c_aimhelper::select_multipoint(animation, hitbox, hitgroup_head, scale_hitbox);
		
		points.emplace_back(box.value(), 0.f, animation, false, box.value(), 0.f, 0.f, hitbox, hitgroup_head);

		const auto low_hitchance = pos.has_value();

		for (auto& point : points)
		{
			if (point.rs > 0.f && low_hitchance)
				continue;

			const auto wall = trace_system->wall_penetration(pos.value_or(local->get_shoot_position()),
				point.position, animation);

			if (!wall.has_value())
				continue;

			if (hitbox == c_cs_player::hitbox::head && hitbox != wall.value().hitbox)
				continue;

			point.hitgroup = wall.value().hitgroup;

			if (hitbox == c_cs_player::hitbox::upper_chest
				&& (wall.value().hitbox == c_cs_player::hitbox::head || wall.value().hitbox == c_cs_player::hitbox::neck))
				continue;

			point.damage = wall.value().damage;

			if (point.damage > best_match.damage)
				best_match = point;
		}
	};

	std::vector<c_cs_player::hitbox> hitboxes_scan;

	auto should_head_neck_only = animation->didshot;

	auto should_override_head_aim = cfg.override_head_aim_only;

	if (should_head_neck_only && should_override_head_aim)
	{
		if (should_baim)
			should_head_neck_only = false;

		//todo other conditions.
	}

	if (should_head_neck_only && cfg.head_aim_only_while_firing) // run our only head/neck hitscan
	{
		if (hitboxes_scan.size() > 0)
			hitboxes_scan.clear();

		if (cfg.on_shot_hitscan_head)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::head);
			//hitboxes_scan.push_back(c_cs_player::hitbox::neck);
		}


		if (cfg.on_shot_hitscan_body)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::upper_chest);
			hitboxes_scan.push_back(c_cs_player::hitbox::chest);
			hitboxes_scan.push_back(c_cs_player::hitbox::thorax);
			hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);
		}
	}
	else if (should_baim) // run our standard body_aim hitscan
	{
		if (hitboxes_scan.size() > 0)
			hitboxes_scan.clear();

		if (is_moving ? cfg.hitscan_chest_moving : cfg.hitscan_chest_moving)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::upper_chest);
			//hitboxes_scan.push_back(c_cs_player::hitbox::chest);//i dont like this
		}

		if (is_moving ? cfg.hitscan_stomach_moving : cfg.hitscan_stomach)
			hitboxes_scan.push_back(c_cs_player::hitbox::thorax);

		if (is_moving ? cfg.hitscan_pelvis_moving : cfg.hitscan_pelvis)
			hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);

		if (is_moving ? cfg.hitscan_legs_moving : cfg.hitscan_legs)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::left_thigh);
			hitboxes_scan.push_back(c_cs_player::hitbox::right_thigh);
		}

		if (is_moving ? cfg.hitscan_feet_moving : cfg.hitscan_feet)
		{
			//hitboxes_scan.push_back(c_cs_player::hitbox::left_calf);//lets not waste ressources
			//hitboxes_scan.push_back(c_cs_player::hitbox::right_calf);
			hitboxes_scan.push_back(c_cs_player::hitbox::left_foot);
			hitboxes_scan.push_back(c_cs_player::hitbox::right_foot);
		}

	}
	else // run our normal hitscan
	{
		if (hitboxes_scan.size() > 0)
			hitboxes_scan.clear();

		if (is_moving ? cfg.hitscan_head_moving : cfg.hitscan_head)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::head);
			//hitboxes_scan.push_back(c_cs_player::hitbox::neck);//dont need 1000 scans
		}

		if (is_moving ? cfg.hitscan_chest_moving : cfg.hitscan_chest)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::upper_chest);
			//hitboxes_scan.push_back(c_cs_player::hitbox::chest);//i dont like this
		}

		if (is_moving ? cfg.hitscan_stomach_moving : cfg.hitscan_stomach)
			hitboxes_scan.push_back(c_cs_player::hitbox::thorax);

		if (is_moving ? cfg.hitscan_pelvis_moving : cfg.hitscan_pelvis)
			hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);

		if (is_moving ? cfg.hitscan_legs_moving : cfg.hitscan_legs)
		{
			hitboxes_scan.push_back(c_cs_player::hitbox::left_thigh);
			hitboxes_scan.push_back(c_cs_player::hitbox::right_thigh);
		}

		if (is_moving ? cfg.hitscan_feet_moving : cfg.hitscan_feet)
		{
			//hitboxes_scan.push_back(c_cs_player::hitbox::left_calf);//lets not waste ressources
			//hitboxes_scan.push_back(c_cs_player::hitbox::right_calf);
			hitboxes_scan.push_back(c_cs_player::hitbox::left_foot);
			hitboxes_scan.push_back(c_cs_player::hitbox::right_foot);
		}
	}

	auto can_fire_at_hitbox = false;


	if (animation->didshot || animation->sideways) {//cuz we can
		hitboxes_scan.clear(); 
		hitboxes_scan.push_back(c_cs_player::hitbox::head);
		hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);
	}
		

	//if(!hitboxes_scan.size())
		//hitboxes_scan.push_back(c_cs_player::hitbox::head);//scan head or we wont do anything jesus

	for (const auto& hitbox : hitboxes_scan) //do our hitscan for our hitboxes
		scan_box(hitbox);

	//check if we can hit the hitbox!

	

	if (weapon_cfg.value().min_dmg_hp && animation->player->get_health() <= weapon_cfg.value().hp || animation->player->get_health() < weapon_cfg.value().min_dmg)
	{
		if (best_match.damage >= animation->player->get_health() + cfg.min_dmg_hp_slider || best_match.damage - 10.f >= static_cast<float>(animation->player->get_health()))
		{
			can_fire_at_hitbox = true;
		}
	}
	else if (animation->player->get_health() <= weapon_cfg.value().hp_health_override)
	{
		if (best_match.damage >= cfg.hp_mindmg || best_match.damage - 10.f >= static_cast<float>(animation->player->get_health()))
		{
			can_fire_at_hitbox = true;
		}
	}

	else if (!(weapon_cfg.value().min_dmg_hp) && !(animation->player->get_health() <= weapon_cfg.value().hp) || !(animation->player->get_health() < weapon_cfg.value().min_dmg))
	{
		if (best_match.damage >= cfg.min_dmg || best_match.damage - 10.f >= static_cast<float>(animation->player->get_health()))
		{
			can_fire_at_hitbox = true;
		}
	}
	else if (can_fire_at_hitbox == false)
	{ 
		//aimbot cannot fire at the player lets figure out why?
		//lets try head :)
		//scan_box(c_cs_player::hitbox::head); //lets hitscan the head only! :)

		if (weapon_cfg.value().min_dmg_hp && animation->player->get_health() <= weapon_cfg.value().hp || animation->player->get_health() < weapon_cfg.value().min_dmg)
		{
			if (best_match.damage >= animation->player->get_health() + cfg.min_dmg_hp_slider || best_match.damage - 10.f >= static_cast<float>(animation->player->get_health()))
				can_fire_at_hitbox = true;
		}
		else
		{
			if (best_match.damage >= cfg.min_dmg || best_match.damage - 10.f >= static_cast<float>(animation->player->get_health()))
				can_fire_at_hitbox = true;
		}
	}

	if (can_fire_at_hitbox == true)
		return best_match;
	else
		return std::nullopt;
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record_knife(c_cs_player * local, c_animation_system::animation* animation)
{
	static const auto is_behind = [] (c_cs_player* local, c_animation_system::animation* animation) -> bool
	{
		auto vec_los = animation->origin - local->get_origin();
		vec_los.z = 0.0f;

		c_vector3d forward;
		math::angle_vectors(animation->eye_angles, forward);
		forward.z = 0.0f;

		return vec_los.normalize().dot(forward) > 0.475f;
	};

	static const auto should_stab = [] (c_cs_player* local, c_animation_system::animation* animation) -> bool
	{
		struct table_t
		{
			unsigned char swing[2][2][2];
			unsigned char stab[2][2];
		};

		static const table_t table = {
			{
				{
					{ 25, 90 },
					{ 21, 76 }
				},
				{
					{ 40, 90 },
					{ 34, 76 }
				}
			},
			{
				{ 65, 180 },
				{ 55, 153 }
			}
		};

		const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
			client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

		if (!weapon)
			return false;

		const auto has_armor = animation->player->get_armor() > 0;
		const auto first_swing = weapon->get_next_primary_attack() + 0.4f < global_vars_base->curtime;
		const auto behind = is_behind(local, animation);

		const int stab_dmg = table.stab[has_armor][behind];
		const int slash_dmg = table.swing[false][has_armor][behind];
		const int swing_dmg = table.swing[first_swing][has_armor][behind];

		if (animation->player->get_health() <= swing_dmg)
			return false;

		if (animation->player->get_health() <= stab_dmg)
			return true;

		if (animation->player->get_health() > swing_dmg + slash_dmg + stab_dmg)
			return true;

		return false;
	};

	const auto studio_model = model_info_client()->get_studio_model(animation->player->get_model());

	if (!studio_model)
		return std::nullopt;

	const auto stab = should_stab(local, animation);
	const auto range = stab ? 32.0f : 48.0f;
	game_trace tr;
	auto spot = animation->player->get_hitbox_position(c_cs_player::hitbox::upper_chest, animation->bones);
	const auto hitbox = studio_model->get_hitbox(static_cast<uint32_t>(c_cs_player::hitbox::upper_chest), 0);

	if (!spot.has_value() || !hitbox)
		return std::nullopt;

	c_vector3d forward;
	const auto calc = math::calc_angle(local->get_shoot_position(), spot.value());
	math::angle_vectors(calc, forward);

	spot.value() += forward * hitbox->radius;

	c_trace_system::run_emulated(animation, [&] () -> void
	{
		uint32_t filter[4] = { c_engine_trace::get_filter_simple_vtable(), reinterpret_cast<uint32_t>(local), 0, 0 };

		ray r;
		c_vector3d aim;
		const auto angle = math::calc_angle(local->get_shoot_position(), spot.value());
		math::angle_vectors(angle, aim);
		const auto end = local->get_shoot_position() + aim * range;
		r.init(local->get_shoot_position(), end);

		engine_trace()->trace_ray(r, mask_solid, reinterpret_cast<trace_filter*>(filter), &tr);

		if (tr.fraction >= 1.0f)
		{
			const c_vector3d min(-16.f, -16.f, -18.f);
			const c_vector3d max(16.f, 16.f, 18.f);
			r.init(local->get_shoot_position(), end, min, max);
			engine_trace()->trace_ray(r, mask_solid, reinterpret_cast<trace_filter*>(filter), &tr);
		}
	});

	if (tr.entity != animation->player)
		return std::nullopt;

	return aim_info { tr.endpos, 100.f, animation, stab, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record_gun(c_cs_player* local, c_animation_system::animation* animation, std::optional<c_vector3d> pos)
{
	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (!animation || !animation->player || !weapon_cfg.has_value())
		return std::nullopt;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return std::nullopt;

	const auto info = animation_system->get_animation_info(animation->player);

	if (!info)
		return std::nullopt;

	const auto cfg = weapon_cfg.value();

	auto should_baim = false;
	
	const auto center = animation->player->get_hitbox_position(c_cs_player::hitbox::pelvis, animation->bones);
	
	if (center.has_value())
	{
		const auto center_wall = trace_system->wall_penetration(pos.value_or(local->get_shoot_position()), center.value(), animation);

		if (center_wall.has_value() && center_wall.value().hitbox == c_cs_player::hitbox::pelvis && center_wall.value().damage - 10.f > animation->player->get_health())
			should_baim = true;
	}

	aim_info best_match = { c_vector3d(), -FLT_MAX, nullptr, false, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };

	const auto scan_box = [&](c_cs_player::hitbox hitbox)
	{
		auto box = animation->player->get_hitbox_position(hitbox, const_cast<matrix3x4*>(animation->bones));

		if (!box.has_value())
			return;

		auto scale_hitbox = 0.f;

		switch (hitbox)
		{
			case c_cs_player::hitbox::head:
				scale_hitbox = cfg.head_scale / 100.f;
			break;
			case c_cs_player::hitbox::neck:
				scale_hitbox = cfg.head_scale / 100.f;
				break;
			case c_cs_player::hitbox::upper_chest:
				scale_hitbox = cfg.chest_scale / 100.f;
				break;
			case c_cs_player::hitbox::chest:
				scale_hitbox = cfg.chest_scale / 100.f;
				break;
			case c_cs_player::hitbox::thorax:
				scale_hitbox = cfg.stomach_scale / 100.f;
				break;
			case c_cs_player::hitbox::pelvis:
				scale_hitbox = cfg.pelvis_scale / 100.f;
				break;
			case c_cs_player::hitbox::left_thigh:
				scale_hitbox = cfg.legs_scale / 100.f;
				break;
			case c_cs_player::hitbox::right_thigh:
				scale_hitbox = cfg.legs_scale / 100.f;
				break;
		}

		auto points = pos.has_value() ? std::vector<aim_info>() : c_aimhelper::select_multipoint(animation, hitbox, hitgroup_head, scale_hitbox);
		
		points.emplace_back(box.value(), 0.f, animation, false, box.value(), 0.f, 0.f, hitbox, hitgroup_head);
		
		for (auto& point : points)
		{
			const auto wall = trace_system->wall_penetration(pos.value_or(local->get_shoot_position()), point.position, animation);

			if (!wall.has_value())
				continue;

			if (hitbox == c_cs_player::hitbox::head && hitbox != wall.value().hitbox)
				continue;

			point.hitgroup = wall.value().hitgroup;

			if (hitbox == c_cs_player::hitbox::upper_chest && (wall.value().hitbox == c_cs_player::hitbox::head || wall.value().hitbox == c_cs_player::hitbox::neck))
				continue;

			point.damage = wall.value().damage;

			if (point.damage > best_match.damage)
				best_match = point;
		}
	};

	if (should_baim)
		for (const auto& hitbox : c_cs_player::hitboxes_baim)
			scan_box(hitbox);
	else
		for (const auto& hitbox : c_cs_player::hitboxes_aiming)
			scan_box(hitbox);



	if (weapon_cfg.value().min_dmg_hp && animation->player->get_health() <= weapon_cfg.value().hp || animation->player->get_health() < weapon_cfg.value().min_dmg)
	{

		if (best_match.damage >= animation->player->get_health() + cfg.min_dmg_hp_slider || best_match.damage - 10.f >= animation->player->get_health())
			return best_match;
	}
	else if (animation->player->get_health() <= weapon_cfg.value().hp_health_override)
	{
		if (best_match.damage >= cfg.hp_mindmg || best_match.damage - 10.f >= animation->player->get_health())
			return best_match;
	}
	else if (!weapon_cfg.value().min_dmg_hp && !(animation->player->get_health() <= weapon_cfg.value().hp) || !(animation->player->get_health() < weapon_cfg.value().min_dmg))
	{
		if (best_match.damage >= cfg.min_dmg || best_match.damage - 10.f >= animation->player->get_health())
			return best_match;
	}
	//else if (should_baim)
		//scan_box(c_cs_player::hitbox::head);

	return std::nullopt;
}

c_ragebot::autostop_info& c_ragebot::get_autostop_info(c_user_cmd *cmd)
{
	if (cmd->buttons & ~c_user_cmd::jump)
	{
		static autostop_info info{ -FLT_MAX, false };
		return info;
	}

	c_ragebot::autostop_info stop;

	return stop;
}

bool c_ragebot::is_breaking_lagcomp(c_animation_system::animation* animation)
{
	static constexpr auto teleport_dist = 64 * 64;

	const auto info = animation_system->get_animation_info(animation->player);

	if (!info || info->frames.size() < 2)
		return false;

	if (info->frames[0].dormant)
		return false;

	auto prev_org = info->frames[0].origin;
	auto skip_first = true;

	// walk context looking for any invalidating event
	for (auto& record : info->frames)
	{
		if (skip_first)
		{
			skip_first = false;
			continue;
		}

		if (record.dormant)
			break;

		auto delta = record.origin - prev_org;
		if (delta.length2dsqr() > teleport_dist)
		{
			// lost track, too much difference
			return true;
		}

		// did we find a context smaller than target time?
		if (record.sim_time <= animation->sim_time)
			break; // hurra, stop

		prev_org = record.origin;
	}
	return false;
}
