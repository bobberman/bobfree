#include "c_resolver.h"
#include "../utils/math.h"
#include "c_aimhelper.h"
#include "c_trace_system.h"
#include "c_ragebot.h"
#include "../sdk/c_debug_overlay.h"
#include "../sdk/c_client_entity_list.h"
#include <random>
#include "c_esp.h"

static std::random_device rd;
static std::mt19937 rng(rd());



void c_resolver::resolve(c_animation_system::animation* anim)
{
	auto is_moving = anim->player->get_velocity().length2d() > 0.1 && anim->player->is_on_ground();

	const auto info = animation_system->get_animation_info(anim->player);

	if (!info || !anim->has_anim_state)
		return;

	const auto slow_walk = anim->anim_state.feet_yaw_rate >= 0.01f && anim->anim_state.feet_yaw_rate <= 0.8f;

	if (info->brute_state == resolver_start)
		info->brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -130.f : 130.f; 
		//info->brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -130.f : 130.f; // original

	if (info->brute_moving_state == resolver_moving_start)
		info->moving_brute_yaw = slow_walk ? std::uniform_int_distribution<int>(0, 1) (rng) ? -150.f : 150.f : 0.f;
	
	if (info->brute_moving_state == resolver_moving_start_inverse)
		info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -167.f : 167.f;




	if (is_moving)
		anim->anim_state.goal_feet_yaw = math::normalize_yaw(anim->anim_state.goal_feet_yaw + info->moving_brute_yaw); // set to 0 at start :)
	else
		anim->anim_state.goal_feet_yaw = math::normalize_yaw(anim->anim_state.goal_feet_yaw + info->brute_yaw);
}

void c_resolver::resolve_shot(resolver::shot& shot)
{
	if (!config.rage.enabled || shot.manual)
		return;

	const auto player = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(shot.record.index));

	if (player != shot.record.player)
		return;

	const auto hdr = model_info_client()->get_studio_model(shot.record.player->get_model());

	if (!hdr)
		return;

	const auto info = animation_system->get_animation_info(player);

	if (!info)
		return;

	const auto angle = math::calc_angle(shot.start, shot.server_info.impacts.back());
	c_vector3d forward;
	math::angle_vectors(angle, forward);

	const auto end = shot.server_info.impacts.back() + forward * 2000.f;
	const auto spread_miss = !c_aimhelper::can_hit_hitbox(shot.start, end, &shot.record, hdr, shot.hitbox);

	if (shot.server_info.damage > 0)
	{
		static const auto hit_msg = __("%d in %s to %s [index: %i] [history ticks: %i] | DidShot %s, UpPitch %s, Sideways %s.");
		_rt(hit, hit_msg);
		char msg[255];

		switch (shot.server_info.hitgroup)
		{
		case hitgroup_generic:
			sprintf_s(msg, hit, shot.server_info.damage, _("generic"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_head:
			sprintf_s(msg, hit, shot.server_info.damage, _("head"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_chest:
			sprintf_s(msg, hit, shot.server_info.damage, _("chest"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_stomach:
			sprintf_s(msg, hit, shot.server_info.damage, _("stomach"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_leftarm:
			sprintf_s(msg, hit, shot.server_info.damage, _("left arm"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_rightarm:
			sprintf_s(msg, hit, shot.server_info.damage, _("right arm"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_leftleg:
			sprintf_s(msg, hit, shot.server_info.damage, _("left leg"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_rightleg:
			sprintf_s(msg, hit, shot.server_info.damage, _("right leg"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		case hitgroup_gear:
			sprintf_s(msg, hit, shot.server_info.damage, _("gear"), player->get_info().name, shot.server_info.index, shot.tickcount, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
			break;
		}

		logging->info(msg);
	}
	else if (spread_miss)
	{
		char msg[255];
		static const auto hit_msg = __("Missed shot due to spread (config issue). Shot %s, Up %s, Sidew %s.");
		_rt(hit, hit_msg);
		sprintf_s(msg, hit, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));
		
		logging->info(msg);

		//c_esp::AddShot(player, 1);
		++info->missed_due_to_spread;
	}
	else
	{
		char msg[255];
		static const auto hit_msg = __("Missed shot due to resolver. Target shot %s, Up %s, Sidew %s.");
		_rt(hit, hit_msg);
		sprintf_s(msg, hit, shot.shotting ? _("Yes") : _("No"), shot.uppitch ? _("Yes") : _("No"), shot.sideways ? _("Yes") : _("No"));

		logging->info(msg);

		//c_esp::AddShot(player, 1);
		++info->missed_due_to_resolver;
	}

	if (!shot.record.player->is_alive() || !shot.record.has_anim_state || !shot.record.player->get_anim_state() || !info)
		return;

	// note old brute_yaw.
	const auto old_brute_yaw = info->brute_yaw;

	// check deviation from server.
	auto backup = c_animation_system::animation(shot.record.player);
	shot.record.apply(player);

	const auto trace = trace_system->wall_penetration(shot.start, end, &shot.record);

	auto does_match = (trace.has_value() && trace.value().hitgroup == shot.server_info.hitgroup) || (!trace.has_value() && spread_miss);

	auto is_moving = shot.record.player->get_velocity().length2d() > 0.1f && shot.record.player->is_on_ground();

	if (info->missed_due_to_resolver >= is_moving ? 5 : 4)
		info->missed_due_to_resolver = 0;
	if (!does_match) {
		char tekt[123];
		sprintf(tekt, _("does not match!!! %d"), time_to_ticks(global_vars_base->curtime));
		logging->info(tekt);
	}
	bool brute_once = false;
	// start brute.
	if (!does_match) // this is fucking shit raxer
	{
		brute_once = true;
		if (is_moving) 
		{
			switch (info->brute_moving_state)
			{
			case resolver_moving_start:
				info->brute_moving_state = resolver_moving_start_inverse;
				break;
			case resolver_moving_start_inverse:
				info->brute_moving_state = resolver_moving_inverse;
				info->moving_brute_yaw = -info->brute_yaw;
				break;
			case resolver_moving_inverse:
				info->brute_moving_state = resolver_moving_no_desync;
				info->moving_brute_yaw = 0.f;
				break;
			case resolver_moving_no_desync:
				info->brute_moving_state = resolver_moving_jitter;
				info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_moving_jitter:
				info->brute_moving_state = resolver_moving_start;
				info->moving_brute_yaw = -info->moving_brute_yaw;
				break;
			}
		}
		else
		{
			switch (info->brute_state)
			{
			case resolver_start:
				info->brute_state = resolver_inverse;
				info->brute_yaw = -info->brute_yaw;
				break;
			case resolver_inverse:
				info->brute_state = resolver_no_desync;
				info->brute_yaw = 0.f;
				break;
			case resolver_no_desync:
				info->brute_state = resolver_jitter;
				info->brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_jitter:
				info->brute_state = resolver_start;
				info->brute_yaw = -info->brute_yaw;
				break;
			}
		}
	}
	if (!brute_once && !shot.server_info.damage > 0) {
		char tekt[123];
		sprintf(tekt, _("damage <= 0!!! %d"), time_to_ticks(global_vars_base->curtime));
		logging->info(tekt);
	}
		
	if (!brute_once && !shot.server_info.damage > 0)
	{
		if (is_moving)
		{
			switch (info->missed_due_to_resolver)
			{
			case resolver_moving_start:
				info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -165.f : 165.f;
				break;
			case resolver_moving_start_inverse:
				info->moving_brute_yaw = -info->brute_yaw;
				break;
			case resolver_moving_inverse:
				info->moving_brute_yaw = 0.f;
				break;
			case resolver_moving_no_desync:
				info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_moving_jitter:
				info->moving_brute_yaw = -info->moving_brute_yaw;
				break;
			}
		}
		else 
		{
			switch (info->missed_due_to_resolver)
			{
			case resolver_start:
				info->brute_yaw = -info->brute_yaw;
				break;
			case resolver_inverse:
				info->brute_yaw = 0.f;
				break;
			case resolver_no_desync:
				info->brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_jitter:
				info->brute_yaw = -info->brute_yaw;
				break;
			}
		}
	}

	// apply changes.
	if (!info->frames.empty())
	{
		c_animation_system::animation* previous = nullptr;

		// jump back to the beginning.
		*player->get_anim_state() = info->frames.back().anim_state;

		for (auto it = info->frames.rbegin(); it != info->frames.rend(); ++it)
		{
			auto& frame = *it;

			const auto frame_player = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(frame.index));

			if (frame_player == frame.player && frame.player == player)
			{
				// re-run complete animation code and repredict all animations in between!
				frame.anim_state = *player->get_anim_state();
				frame.anim_state.goal_feet_yaw = is_moving ? info->moving_brute_yaw : info->brute_yaw; //added cuz we never use movingbruteyaw 24.july

				frame.apply(player);

				player->get_flags() = frame.flags;
				*player->get_animation_layers() = frame.layers;
				player->get_simtime() = frame.sim_time;

				info->update_animations(&frame, previous);

				frame.abs_ang.y = player->get_anim_state()->goal_feet_yaw;
				frame.flags = player->get_flags();

				*player->get_animation_layers() = frame.layers;

				frame.build_server_bones(player);

				previous = &frame;
			}
		}
	}
}

void c_resolver::register_shot(resolver::shot&& s)
{
	shots.emplace_front(std::move(s));
}

void c_resolver::on_player_hurt(c_game_event* event)
{
	const auto attacker = event->get_int(_("attacker"));
	const auto attacker_index = engine_client()->get_player_for_user_id(attacker);

	if (attacker_index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (it->confirmed && !it->skip)
		{
			last_confirmed = &*it;
			break;
		}
	}

	if (!last_confirmed)
		return;

	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != last_confirmed->record.index)
		return;

	last_confirmed->playerhurt = true;
	last_confirmed->server_info.index = index;
	last_confirmed->server_info.damage = event->get_int(_("dmg_health"));
	last_confirmed->server_info.hitgroup = event->get_int(_("hitgroup"));
}

void c_resolver::on_bullet_impact(c_game_event* event)
{
	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (it->confirmed && !it->skip)
		{
			last_confirmed = &*it;
			break;
		}
	}

	if (!last_confirmed)
		return;

	last_confirmed->impacted = true;

	last_confirmed->server_info.impacts.emplace_back(event->get_float(_("x")),
		event->get_float(_("y")),
		event->get_float(_("z")));
}

void c_resolver::on_weapon_fire(c_game_event* event)
{
	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_unconfirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (!it->confirmed)
		{
			last_unconfirmed = &*it;
			break;
		}

		it->skip = true;
	}

	if (!last_unconfirmed)
		return;

	last_unconfirmed->confirmed = true;
}

void c_resolver::on_render_start()
{
	if (shots.empty())return;
	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->time + 1.f < global_vars_base->curtime)
			it = shots.erase(it);
		else
			it = next(it);
	}

	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->confirmed && it->impacted)
		{
			if (!it->delayed)it->delayed = true;
			else {
				resolve_shot(*it);
				c_esp::draw_local_impact(it->start, it->server_info.impacts.back());
				it = shots.erase(it);
			}
		}
		else
			it = next(it);
	}
}



// beta

void c_resolver_beta::resolve(c_animation_system::animation* anim)
{
	auto is_moving = anim->player->get_velocity().length2d() > 0.1 && anim->player->is_on_ground();

	const auto info = animation_system->get_animation_info(anim->player);

	if (!info || !anim->has_anim_state)
		return;

	const auto slow_walk = anim->anim_state.feet_yaw_rate >= 0.01f && anim->anim_state.feet_yaw_rate <= 0.8f;


	if (is_moving) {
		auto ang = anim->player->get_abs_angles();
		anim->player->set_abs_angles(c_qangle(ang.x, anim->lby, ang.z));
		anim->anim_state.goal_feet_yaw = math::normalize_yaw(anim->anim_state.goal_feet_yaw + info->moving_brute_yaw); // set to 0 at start :)
	}
	else
		anim->anim_state.goal_feet_yaw = math::normalize_yaw(anim->anim_state.goal_feet_yaw + info->brute_yaw);
}

void c_resolver_beta::resolve_shot(resolver::shot& shot)
{
	if (!config.rage.enabled || shot.manual)
		return;

	const auto player = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(shot.record.index));

	if (player != shot.record.player)
		return;

	const auto hdr = model_info_client()->get_studio_model(shot.record.player->get_model());

	if (!hdr)
		return;

	const auto info = animation_system->get_animation_info(player);

	if (!info)
		return;

	const auto angle = math::calc_angle(shot.start, shot.server_info.impacts.back());
	c_vector3d forward;
	math::angle_vectors(angle, forward);

	const auto end = shot.server_info.impacts.back() + forward * 2000.f;
	const auto spread_miss = !c_aimhelper::can_hit_hitbox(shot.start, end, &shot.record, hdr, shot.hitbox);

	if (shot.server_info.damage > 0)
	{
		static const auto hit_msg = __("-%d in %s to %s [index: %i] [history ticks: %i]");
		_rt(hit, hit_msg);
		char msg[255];

		switch (shot.server_info.hitgroup)
		{
		case hitgroup_generic:
			sprintf_s(msg, hit, shot.server_info.damage, _("generic"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_head:
			sprintf_s(msg, hit, shot.server_info.damage, _("head"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_chest:
			sprintf_s(msg, hit, shot.server_info.damage, _("chest"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_stomach:
			sprintf_s(msg, hit, shot.server_info.damage, _("stomach"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_leftarm:
			sprintf_s(msg, hit, shot.server_info.damage, _("left arm"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_rightarm:
			sprintf_s(msg, hit, shot.server_info.damage, _("right arm"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_leftleg:
			sprintf_s(msg, hit, shot.server_info.damage, _("left leg"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_rightleg:
			sprintf_s(msg, hit, shot.server_info.damage, _("right leg"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		case hitgroup_gear:
			sprintf_s(msg, hit, shot.server_info.damage, _("gear"), player->get_info().name, shot.server_info.index, shot.tickcount);
			break;
		}

		logging->info(msg);
	}
	else if (spread_miss)
	{

		logging->info(_("Missed shot due to spread (config issue)"));

		//c_esp::AddShot(player, 1);
		++info->missed_due_to_spread;
	}
	else
	{
		logging->info(_("Missed shot due to resolver"));

		//c_esp::AddShot(player, 1);
		++info->missed_due_to_resolver;
	}




	if (!shot.record.player->is_alive() || !shot.record.has_anim_state || !shot.record.player->get_anim_state() || !info)
		return;

	// note old brute_yaw.
	const auto old_brute_yaw = info->brute_yaw;

	// check deviation from server.
	auto backup = c_animation_system::animation(shot.record.player);
	shot.record.apply(player);

	const auto trace = trace_system->wall_penetration(shot.start, end, &shot.record);

	auto does_match = (trace.has_value() && trace.value().hitgroup == shot.server_info.hitgroup) || (!trace.has_value() && spread_miss);

	auto is_moving = shot.record.player->get_velocity().length2d() > 0.1f && shot.record.player->is_on_ground();

	if (info->missed_due_to_resolver >= is_moving ? 5 : 4)
		info->missed_due_to_resolver = 0;

	// start brute.
	if (!does_match) // this is fucking shit raxer
	{
		if (is_moving)
		{
			switch (info->brute_moving_state)
			{
			case resolver_moving_start:
				info->brute_moving_state = resolver_moving_start_inverse;
				break;
			case resolver_moving_start_inverse:
				info->brute_moving_state = resolver_moving_inverse;
				info->moving_brute_yaw = -info->brute_yaw;
				break;
			case resolver_moving_inverse:
				info->brute_moving_state = resolver_moving_no_desync;
				info->moving_brute_yaw = 0.f;
				break;
			case resolver_moving_no_desync:
				info->brute_moving_state = resolver_moving_jitter;
				info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_moving_jitter:
				info->brute_moving_state = resolver_moving_start;
				info->moving_brute_yaw = -info->moving_brute_yaw;
				break;
			}
		}
		else
		{
			switch (info->brute_state)
			{
			case resolver_start:
				info->brute_state = resolver_inverse;
				info->brute_yaw = -info->brute_yaw;
				break;
			case resolver_inverse:
				info->brute_state = resolver_no_desync;
				info->brute_yaw = 0.f;
				break;
			case resolver_no_desync:
				info->brute_state = resolver_jitter;
				info->brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_jitter:
				info->brute_state = resolver_start;
				info->brute_yaw = -info->brute_yaw;
				break;
			}
		}
	}

	if (!shot.server_info.damage > 0)
	{
		if (is_moving)
		{
			switch (info->missed_due_to_resolver)
			{
			case resolver_moving_start:
				info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -165.f : 165.f;
				break;
			case resolver_moving_start_inverse:
				info->moving_brute_yaw = -info->brute_yaw;
				break;
			case resolver_moving_inverse:
				info->moving_brute_yaw = 0.f;
				break;
			case resolver_moving_no_desync:
				info->moving_brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_moving_jitter:
				info->moving_brute_yaw = -info->moving_brute_yaw;
				break;
			}
		}
		else
		{
			switch (info->missed_due_to_resolver)
			{
			case resolver_start:
				info->brute_yaw = -info->brute_yaw;
				break;
			case resolver_inverse:
				info->brute_yaw = 0.f;
				break;
			case resolver_no_desync:
				info->brute_yaw = std::uniform_int_distribution<int>(0, 1)(rng) ? -120.f : 120.f;
				break;
			default:
			case resolver_jitter:
				info->brute_yaw = -info->brute_yaw;
				break;
			}
		}
	}

	// apply changes.
	if (!info->frames.empty())
	{
		c_animation_system::animation* previous = nullptr;

		// jump back to the beginning.
		*player->get_anim_state() = info->frames.back().anim_state;

		for (auto it = info->frames.rbegin(); it != info->frames.rend(); ++it)
		{
			auto& frame = *it;

			const auto frame_player = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(frame.index));

			if (frame_player == frame.player && frame.player == player)
			{
				// re-run complete animation code and repredict all animations in between!
				frame.anim_state = *player->get_anim_state();
				frame.anim_state.goal_feet_yaw = info->brute_yaw;

				frame.apply(player);

				player->get_flags() = frame.flags;
				*player->get_animation_layers() = frame.layers;
				player->get_simtime() = frame.sim_time;

				info->update_animations(&frame, previous);

				frame.abs_ang.y = player->get_anim_state()->goal_feet_yaw;
				frame.flags = player->get_flags();

				*player->get_animation_layers() = frame.layers;

				frame.build_server_bones(player);

				previous = &frame;
			}
		}
	}
}

void c_resolver_beta::register_shot(resolver::shot&& s)
{
	shots.emplace_front(std::move(s));
}

void c_resolver_beta::on_player_hurt(c_game_event* event)
{
	const auto attacker = event->get_int(_("attacker"));
	const auto attacker_index = engine_client()->get_player_for_user_id(attacker);

	if (attacker_index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (it->confirmed && !it->skip)
		{
			last_confirmed = &*it;
			break;
		}
	}

	if (!last_confirmed)
		return;

	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != last_confirmed->record.index)
		return;

	last_confirmed->server_info.index = index;
	last_confirmed->server_info.damage = event->get_int(_("dmg_health"));
	last_confirmed->server_info.hitgroup = event->get_int(_("hitgroup"));
}

void c_resolver_beta::on_bullet_impact(c_game_event* event)
{
	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (it->confirmed && !it->skip)
		{
			last_confirmed = &*it;
			break;
		}
	}

	if (!last_confirmed)
		return;

	last_confirmed->impacted = true;
	last_confirmed->server_info.impacts.emplace_back(event->get_float(_("x")),
		event->get_float(_("y")),
		event->get_float(_("z")));
}

void c_resolver_beta::on_weapon_fire(c_game_event* event)
{
	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_unconfirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (!it->confirmed)
		{
			last_unconfirmed = &*it;
			break;
		}

		it->skip = true;
	}

	if (!last_unconfirmed)
		return;

	last_unconfirmed->confirmed = true;
}

void c_resolver_beta::on_render_start()
{
	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->time + 1.f < global_vars_base->curtime)
			it = shots.erase(it);
		else
			it = next(it);
	}

	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->confirmed && it->impacted)
		{
			resolve_shot(*it);
			c_esp::draw_local_impact(it->start, it->server_info.impacts.back());
			it = shots.erase(it);
		}
		else
			it = next(it);
	}
}
