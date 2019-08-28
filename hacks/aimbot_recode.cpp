#include "aimbot_recode.h"
#include "c_aimhelper.h"
#include "c_trace_system.h"
#include "../utils/math.h"
#include "../sdk/c_weapon_system.h"
#include "../sdk/c_debug_overlay.h"
#include "c_prediction_system.h"
#include "c_antiaim.h"
#include "c_resolver.h"
#include "c_movement.h"
#include "c_esp.h"
#include "../menu/c_menu.h"

c_cs_player::hitbox hitboxes;
float damage;
int hitgroup;

void c_aimbot::aimbot_run(c_cs_player* local, c_user_cmd* cmd)
{
	hitboxes = c_cs_player::hitbox::head;
	damage = -0.f;
	hitgroup = -1;
	last_pitch = std::nullopt;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

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

	client_entity_list()->for_each_player([&](c_cs_player* player) -> void
	{
		if (!player->is_enemy() || !player->is_alive() || player->get_gun_game_immunity())
			return;

		if (aimbot_target(player, cmd, local))
			return;
	});
}

bool c_aimbot::aimbot_target(c_cs_player* target, c_user_cmd* cmd, c_cs_player* local)
{
	bool is_autoshooting = false;

	c_vector3d aimbot_bone;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return false;

	const auto wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!wpn_info)
		return false;

	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (!weapon_cfg.has_value())
		return false;

	const auto latest = animation_system->get_latest_animation(target);
	const auto latest_firing = animation_system->get_latest_firing_animation(target);

	if (!latest.has_value())
		return false;

	if (latest_firing.has_value())
	{
		

		
		if (weapon_cfg.value().min_dmg_hp && target->get_health() <= weapon_cfg.value().hp || target->get_health() < weapon_cfg.value().min_dmg)
		{
			aimbot_bone = aimbot_get_bone(target, local, c_cs_player::hitbox::head, target->get_health() + weapon_cfg.value().min_dmg_hp_slider, false, latest_firing.value());
		}
		else if (target->get_health() <= weapon_cfg.value().hp_health_override)
		{
			aimbot_bone = aimbot_get_bone(target, local, c_cs_player::hitbox::head, weapon_cfg.value().hp_mindmg, false, latest_firing.value());
		}
		else
		{
			aimbot_bone = aimbot_get_bone(target, local, c_cs_player::hitbox::head, weapon_cfg.value().min_dmg, false, latest_firing.value());
		}

		// Invalid target/no hitable points at all.
		if (!aimbot_bone.is_valid())
			return false;

		debug_overlay()->add_line_overlay(local->get_shoot_position(), aimbot_bone, 255, 0, 0, true, 0.1);

		// scope the weapon.
		if ((wpn_info->get_weapon_id() == weapon_g3sg1 || wpn_info->get_weapon_id() == weapon_scar20 || wpn_info->get_weapon_id() == weapon_ssg08 || wpn_info->get_weapon_id() == weapon_awp
			|| wpn_info->get_weapon_id() == weapon_sg556 || wpn_info->get_weapon_id() == weapon_aug) && weapon->get_zoom_level() == 0)
			cmd->buttons |= c_user_cmd::flags::attack2;

		// calculate angle.
		const auto angle = math::calc_angle(local->get_shoot_position(), aimbot_bone);

		// store pitch for eye correction.
		last_pitch = angle.x;


		if (c_aimhelper::is_hitbox_a_body(hitboxes))
		{
			if (!c_aimhelper::can_hit(local, latest.value(), aimbot_bone, weapon_cfg.value().hitchance_body / 100.f, hitboxes))
				return false;
		}
		else
			if (!c_aimhelper::can_hit(local, latest.value(), aimbot_bone, weapon_cfg.value().hitchance_head / 100.f, hitboxes))
				return false;

		resolver::shot shot{};
		shot.damage = damage;
		shot.start = local->get_shoot_position();
		shot.end = aimbot_bone;
		shot.hitgroup = hitgroup;
		shot.hitbox = hitboxes;
		shot.time = global_vars_base->curtime;
		shot.record = *latest_firing.value();
		shot.manual = false;
		shot.tickcount = cmd->tick_count - time_to_ticks(latest_firing.value()->sim_time) + time_to_ticks(calculate_lerp());
		

		switch (config.rage.resolver) {
		case 1: c_resolver::register_shot(std::move(shot));
			break;
		case 2: c_resolver_beta::register_shot(std::move(shot));
			break;
		}


		// set correct information to user_cmd.
		cmd->viewangles = angle;
		cmd->tick_count = time_to_ticks(latest_firing.value()->sim_time) + time_to_ticks(calculate_lerp());

		if (weapon_cfg.value().auto_shoot && config.rage.fake_duck)
		{
			if (local->get_duck_amount() == 0.f)
			{
				cmd->buttons |= c_user_cmd::attack;
				cmd->buttons |= ~c_user_cmd::use;
				bool is_autoshooting = true;
				//c_esp::AddShot(target, 0);
			}
		}

		if (weapon_cfg.value().auto_shoot && !config.rage.fake_duck)
		{
			cmd->buttons |= c_user_cmd::attack;
			cmd->buttons |= ~c_user_cmd::use;
			bool is_autoshooting = true;
			//c_esp::AddShot(target, 0);
		}

		 
	}
	else if (latest.has_value() && !latest.value()->didshot)
	{


		if (weapon_cfg.value().min_dmg_hp && target->get_health() <= weapon_cfg.value().hp || target->get_health() < weapon_cfg.value().min_dmg)
		{
			aimbot_bone = aimbot_get_bone(target, local, c_cs_player::hitbox::head, target->get_health() + weapon_cfg.value().min_dmg_hp_slider, false, latest_firing.value());
		}
		else if (target->get_health() <= weapon_cfg.value().hp_health_override)
		{
			aimbot_bone = aimbot_get_bone(target, local, c_cs_player::hitbox::head, weapon_cfg.value().hp_mindmg, false, latest_firing.value());
		}
		else
		{
		aimbot_bone = aimbot_get_bone(target, local, c_cs_player::hitbox::head, weapon_cfg.value().min_dmg, false, latest_firing.value());
		}

		// Invalid target/no hitable points at all.
		if (!aimbot_bone.is_valid())
			return false;

		debug_overlay()->add_line_overlay(local->get_shoot_position(), aimbot_bone, 255, 0, 0, true, 0.1);

		// scope the weapon.
		if ((wpn_info->get_weapon_id() == weapon_g3sg1 || wpn_info->get_weapon_id() == weapon_scar20 || wpn_info->get_weapon_id() == weapon_ssg08 || wpn_info->get_weapon_id() == weapon_awp
			|| wpn_info->get_weapon_id() == weapon_sg556 || wpn_info->get_weapon_id() == weapon_aug) && weapon->get_zoom_level() == 0)
			cmd->buttons |= c_user_cmd::flags::attack2;

		// calculate angle.
		const auto angle = math::calc_angle(local->get_shoot_position(), aimbot_bone);

		// store pitch for eye correction.
		last_pitch = angle.x;


		if (c_aimhelper::is_hitbox_a_body(hitboxes))
		{
			if (!c_aimhelper::can_hit(local, latest.value(), aimbot_bone,  weapon_cfg.value().hitchance_body / 100.f, hitboxes))
				return false;
		}
		else
			if (!c_aimhelper::can_hit(local, latest.value(), aimbot_bone,  weapon_cfg.value().hitchance_head / 100.f, hitboxes))
				return false;
		
		resolver::shot shot{};
		shot.damage = damage;
		shot.start = local->get_shoot_position();
		shot.end = aimbot_bone;
		shot.hitgroup = hitgroup;
		shot.hitbox = hitboxes;
		shot.time = global_vars_base->curtime;
		shot.record = *latest.value();
		shot.manual = false;
		shot.tickcount = cmd->tick_count - time_to_ticks(latest.value()->sim_time) + time_to_ticks(calculate_lerp());

		// set correct information to user_cmd.
		cmd->viewangles = angle;
		cmd->tick_count = time_to_ticks(latest.value()->sim_time + calculate_lerp());

		if (weapon_cfg.value().auto_shoot && config.rage.fake_duck)
		{
			if (local->get_duck_amount() == 0.f)
			{
				cmd->buttons |= c_user_cmd::attack;
				cmd->buttons |= ~c_user_cmd::use;
				bool is_autoshooting = true;
				//c_esp::AddShot(target, 0);
			}
		}

		if (weapon_cfg.value().auto_shoot && !config.rage.fake_duck)
		{
			cmd->buttons |= c_user_cmd::attack;
			cmd->buttons |= ~c_user_cmd::use;			
			bool is_autoshooting = true;
			//c_esp::AddShot(target, 0);
		}

	}

	return true;
}

c_vector3d c_aimbot::aimbot_get_bone(c_cs_player *player, c_cs_player* local, c_cs_player::hitbox prioritized, float minDmg, bool onlyPrioritized, c_animation_system::animation* animation)
{
	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));
	const auto wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition());
	const auto weapon_cfg = c_aimhelper::get_weapon_conf();
	const auto model = animation->player->get_model();
	const auto studio_model = model_info_client()->get_studio_model(model);
	const auto studio_box = studio_model->get_hitbox_set(player->m_nHitboxSet());
	const auto info = animation_system->get_animation_info(animation->player);

	const auto slow_walk = animation->anim_state.feet_yaw_rate >= 0.01 && animation->anim_state.feet_yaw_rate <= 0.8;
	const auto is_moving = animation->velocity.length2d() > 0.1f && animation->player->is_on_ground() && !slow_walk;

	auto should_baim = false;

	c_vector3d vecOutput(NAN, NAN, NAN);

	std::vector<c_cs_player::hitbox> hitboxes_scan;

	if (!should_baim)
	{
		if (animation->player->get_health() <= weapon_cfg.value().body_aim_health)
			should_baim = true;

		if (weapon_cfg.value().smart_aim)
		{
			if (animation->player->get_health() <= 50)
				should_baim = true;

			if (animation->player->get_velocity().length2d() > 0.1 && !animation->player->is_on_ground())
				should_baim = true;

			if (slow_walk)
				should_baim = true;
		}

		if (weapon_cfg.value().body_aim_in_air)
			if (animation->player->get_velocity().length2d() > 0.1 && !animation->player->is_on_ground())
				should_baim = true;

		if (weapon_cfg.value().body_aim_lethal && animation->player->get_health() < wpn_info->iDamage)
			should_baim = true;

		if (weapon_cfg.value().body_aim_slow_walk)
			if (slow_walk)
				should_baim = true;

		if (GetAsyncKeyState(weapon_cfg.value().body_aim_key))
			should_baim = true;

		auto should_head_neck_only = animation->didshot;

		

		if(GetAsyncKeyState(weapon_cfg.value().head_aim_key))
			should_head_neck_only = true;

		if (info->missed_due_to_resolver >= weapon_cfg.value().body_after_x_missed_resolver + 1)
			should_baim = true;

		if (info->missed_due_to_spread >= weapon_cfg.value().body_after_x_missed_spread + 1)
			should_baim = true;

		if (weapon_cfg.value().body_aim_if_not_on_shot && !animation->didshot)
			should_baim = true;
	}

	if (aimbot_best_point(player, local, prioritized, minDmg, studio_box, vecOutput, animation) > minDmg && onlyPrioritized)
	{
		hitboxes = prioritized;
		return vecOutput;
	}
	else
	{
		float flHigherDamage = 0.f;

		c_vector3d vecCurVec;

		auto should_head_neck_only = animation->didshot;

		auto should_override_head_aim = weapon_cfg.value().override_head_aim_only;

		if (should_head_neck_only && should_override_head_aim)
		{
			
			if (should_baim && !(GetAsyncKeyState(weapon_cfg.value().head_aim_key)))
				should_head_neck_only = false;

			//todo other conditions.
		}

		if (should_head_neck_only && weapon_cfg.value().head_aim_only_while_firing && (GetAsyncKeyState(weapon_cfg.value().head_aim_key))) // run our only head/neck hitscan
		{
			if (hitboxes_scan.size() > 0)
				hitboxes_scan.clear();

			if (weapon_cfg.value().on_shot_hitscan_head)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::head);
				hitboxes_scan.push_back(c_cs_player::hitbox::neck);
			}
		}
		else if (should_baim) // run our standard body_aim hitscan
		{
			if (hitboxes_scan.size() > 0)
				hitboxes_scan.clear();

			if (is_moving ? weapon_cfg.value().hitscan_chest_moving : weapon_cfg.value().hitscan_chest_moving)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::upper_chest);
				hitboxes_scan.push_back(c_cs_player::hitbox::chest);
			}

			if (is_moving ? weapon_cfg.value().hitscan_stomach_moving : weapon_cfg.value().hitscan_stomach)
				hitboxes_scan.push_back(c_cs_player::hitbox::thorax);

			if (is_moving ? weapon_cfg.value().hitscan_pelvis_moving : weapon_cfg.value().hitscan_pelvis)
				hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);

			if (is_moving ? weapon_cfg.value().hitscan_legs_moving : weapon_cfg.value().hitscan_legs)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::left_thigh);
				hitboxes_scan.push_back(c_cs_player::hitbox::right_thigh);
			}

			if (is_moving ? weapon_cfg.value().hitscan_feet_moving : weapon_cfg.value().hitscan_feet)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::left_calf);
				hitboxes_scan.push_back(c_cs_player::hitbox::right_calf);
				hitboxes_scan.push_back(c_cs_player::hitbox::left_foot);
				hitboxes_scan.push_back(c_cs_player::hitbox::right_foot);
			}

		}
		else // run our normal hitscan
		{
			if (hitboxes_scan.size() > 0)
				hitboxes_scan.clear();

			if (is_moving ? weapon_cfg.value().hitscan_head_moving : weapon_cfg.value().hitscan_head)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::head);
				hitboxes_scan.push_back(c_cs_player::hitbox::neck);
			}

			if (is_moving ? weapon_cfg.value().hitscan_chest_moving : weapon_cfg.value().hitscan_chest)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::upper_chest);
				hitboxes_scan.push_back(c_cs_player::hitbox::chest);
			}

			if (is_moving ? weapon_cfg.value().hitscan_stomach_moving : weapon_cfg.value().hitscan_stomach)
				hitboxes_scan.push_back(c_cs_player::hitbox::thorax);

			if (is_moving ? weapon_cfg.value().hitscan_pelvis_moving : weapon_cfg.value().hitscan_pelvis)
				hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);

			if (is_moving ? weapon_cfg.value().hitscan_legs_moving : weapon_cfg.value().hitscan_legs)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::left_thigh);
				hitboxes_scan.push_back(c_cs_player::hitbox::right_thigh);
			}

			if (is_moving ? weapon_cfg.value().hitscan_feet_moving : weapon_cfg.value().hitscan_feet)
			{
				hitboxes_scan.push_back(c_cs_player::hitbox::left_calf);
				hitboxes_scan.push_back(c_cs_player::hitbox::right_calf);
				hitboxes_scan.push_back(c_cs_player::hitbox::left_foot);
				hitboxes_scan.push_back(c_cs_player::hitbox::right_foot);
			}
		}

		for (const auto& hitbox : hitboxes_scan)
		{
			float flCurDamage = aimbot_best_point(player, local, hitbox, minDmg, studio_box, vecCurVec, animation);

			if (!flCurDamage)
				continue;

			if (flCurDamage > flHigherDamage)
			{
				flHigherDamage = flCurDamage;
				hitboxes = hitbox;
			}
		}
		return vecOutput;
	}
}

float c_aimbot::aimbot_best_point(c_cs_player *player, c_cs_player* local, c_cs_player::hitbox prioritized, float minDmg, mstudiohitboxset_t *hitset, c_vector3d &vecOut, c_animation_system::animation* animation)
{
	mstudiobbox_t *hitbox = hitset->get_hitbox(static_cast<uint32_t>(prioritized));

	if (!hitbox)
		return 0.f;

	const auto cfg = c_aimhelper::get_weapon_conf();

	if (!cfg.has_value())
		return 0.f;

	std::vector<c_vector3d> vecArray;

	float flHigherDamage = 0.f;

	c_vector3d min, max;

	math::vector_transform(hitbox->bbmax, animation->bones[hitbox->bone], max);
	math::vector_transform(hitbox->bbmin, animation->bones[hitbox->bone], min);

	auto center = (min + max) * 0.5f;

	c_qangle curAngles = math::calc_angle(center, local->get_shoot_position());

	c_vector3d forward;
	math::angle_vectors(curAngles, forward);

	c_vector3d right = forward.cross(c_vector3d(0, 0, 1));
	c_vector3d left = c_vector3d(-right.x, -right.y, right.z);

	c_vector3d top = c_vector3d(0, 0, 1);
	c_vector3d bot = c_vector3d(0, 0, -1);

	auto scale_hitbox = 0.f;

	switch (prioritized)
	{
	case c_cs_player::hitbox::head:
		scale_hitbox = cfg.value().head_scale / 100.f;
		break;
	case c_cs_player::hitbox::neck:
		scale_hitbox = cfg.value().head_scale / 100.f;
		break;
	case c_cs_player::hitbox::upper_chest:
		scale_hitbox = cfg.value().chest_scale / 100.f;
		break;
	case c_cs_player::hitbox::chest:
		scale_hitbox = cfg.value().chest_scale / 100.f;
		break;
	case c_cs_player::hitbox::thorax:
		scale_hitbox = cfg.value().stomach_scale / 100.f;
		break;
	case c_cs_player::hitbox::pelvis:
		scale_hitbox = cfg.value().pelvis_scale / 100.f;
		break;
	case c_cs_player::hitbox::left_thigh:
		scale_hitbox = cfg.value().legs_scale / 100.f;
		break;
	case c_cs_player::hitbox::right_thigh:
		scale_hitbox = cfg.value().legs_scale / 100.f;
		break;
	case c_cs_player::hitbox::left_foot:
		scale_hitbox = cfg.value().feet_scale / 100.f;
		break;
	case c_cs_player::hitbox::right_foot:
		scale_hitbox = cfg.value().feet_scale / 100.f;
		break;
	}

	switch (prioritized)
	{
	case c_cs_player::hitbox::head:
		for (auto i = 0; i < 4; ++i)
		{
			vecArray.emplace_back(center);
		}
		vecArray[1] += top * (hitbox->radius * scale_hitbox);
		vecArray[2] += right * (hitbox->radius * scale_hitbox);
		vecArray[3] += left * (hitbox->radius * scale_hitbox);
		break;

	default:

		for (auto i = 0; i < 2; ++i)
		{
			vecArray.emplace_back(center);
		}
		vecArray[0] += right * (hitbox->radius * scale_hitbox);
		vecArray[1] += left * (hitbox->radius * scale_hitbox);
		break;
	}

	for (c_vector3d cur : vecArray)
	{
		const auto wall = trace_system->wall_penetration(local->get_shoot_position(), cur, animation);

		if (!wall.has_value())
			continue;

		if ((wall.value().damage > flHigherDamage) && (wall.value().damage > minDmg))
		{
			hitgroup = wall.value().hitgroup;
			damage = wall.value().damage;

			flHigherDamage = wall.value().damage;
			vecOut = cur;
		}
	}

	return flHigherDamage;

}
