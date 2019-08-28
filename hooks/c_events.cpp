#include "../bobhack_main.h"
#include "../hacks/c_hitmarker.h"
#include "../hacks/c_resolver.h"
#include "../BASS/API.h"
#include "../sdk/c_debug_overlay.h"
#include "../sdk/c_view_beams.h"
#include "../hacks/c_esp.h"
#include "../sdk/c_weapon_system.h"
#include "c_events.h"
#include "../hacks/c_damageesp.h"

void c_events::hook()
{
	static c_events events {};
	game_event_manager()->add_listener_global(&events, false);
}

void c_events::fire_game_event(c_game_event* event)
{
	static auto delay = 0;

	if (delay > 0)
		++delay;

	// FIXME: refactor this into something nice.
	const auto buy_bot = []() -> void
	{
		const auto local = c_cs_player::get_local_player(); if (!local) return;
		const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

		if (!weapon) return;
		const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

		const auto is_autosniper = info->get_weapon_id() == weapon_scar20 || info->get_weapon_id() == weapon_g3sg1;

		if (config.misc.buy_bot_primary == 1)
		{
			if (!is_autosniper)
			{
				engine_client()->clientcmd_unrestricted(_("buy scar20; buy g3sg1; buy primammo;"), 0);
			}
			if (is_autosniper)
			{
				engine_client()->clientcmd_unrestricted(_("buy primammo; buy secmammo;"), 0);
			}

		}
		if (config.misc.buy_bot_primary == 2)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy ssg08; buy primammo;"), 0);
		}
		if (config.misc.buy_bot_primary == 3)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy awp; buy primammo;"), 0);
		}

		if (config.misc.buy_bot_secondary == 1)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy elite; buy secmammo;"), 0);
		}
		if (config.misc.buy_bot_secondary == 2)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy p250; buy secmammo;"), 0);
		}
		if (config.misc.buy_bot_secondary == 3)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy tec9; buy fiveseven; buy secmammo;"), 0);
		}
		if (config.misc.buy_bot_secondary == 4)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy deagle; buy secmammo;"), 0);
		}
		if (config.misc.buy_bot_armor == 1)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy vest;"), 0);
		}
		if (config.misc.buy_bot_armor == 2)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy vest; buy vesthelm;"), 0);
		}
		if (config.misc.buy_bot_grenades)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy molotov; buy incgrenade; buy hegrenade; buy smokegrenade; buy flashbang; "), 0);
		}
		if (config.misc.buy_bot_zeus)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy taser; "), 0);
		}
		if (config.misc.buy_bot_defuser)
		{
			engine_client()->clientcmd_unrestricted(
				_("buy defuser; "), 0);
		}
		
	
	};

	auto run_buy_bot = false;

	if (delay >= 50)
	{
		delay = 0;
		buy_bot();
		run_buy_bot = true;
	}

	switch (fnv1a_rt(event->get_name()))
	{
	case fnv1a("client_disconnect"):
		round_flags = ROUND_STARTING;
		c_esp::clear_dmg();
		break;
	case fnv1a("round_freeze_end"):
		round_flags = ROUND_IN_PROGRESS;
		break;
	case fnv1a("player_hurt"):
		damageesp->on_player_hurt(event);
		c_hitmarker::on_player_hurt(event);
		c_hitmarker::deadsound(event);
		c_esp::adddmg(event);
		switch(config.rage.resolver){
		case 1:c_resolver::on_player_hurt(event);
			break;
		case 2:c_resolver_beta::on_player_hurt(event);
			break;
		}

		break;
	case fnv1a("weapon_fire"):
		switch (config.rage.resolver) {
		case 1:c_resolver::on_weapon_fire(event);
			break;
		case 2:c_resolver_beta::on_weapon_fire(event);
			break;
		}

		break;
	case fnv1a("bullet_impact"):
		damageesp->on_bullet_impact(event);
		c_esp::draw_enemy_impact(event);
		switch (config.rage.resolver) {
		case 1:c_resolver::on_bullet_impact(event);
			break;
		case 2:c_resolver_beta::on_bullet_impact(event);
			break;
		}
		

		c_hitmarker::on_bullet_impact(event);
		break;
	case fnv1a("round_end"):
		c_hitmarker::on_round_start(event);
		is_active_round = false;
		round_flags = ROUND_ENDING;
		break;
	case fnv1a("round_start"):
		round_flags = ROUND_STARTING;
		c_hitmarker::on_round_start(event);
		is_active_round = true;
		delay = 1;
		c_esp::clear_dmg();
		if (!run_buy_bot)
			buy_bot();

		break;
	case fnv1a("player_death"): // for killsay
		break;
	default:
		break;
	}
}

int c_events::get_event_debug_id()
{
	return 42;
}
