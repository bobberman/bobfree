#include "c_damageesp.h"
#include "c_resolver.h"
#include "c_esp.h"
#include "../utils/math.h"
#include "../sdk/c_model_info_client.h"
#include "../utils/c_config.h"
#include "../sdk/c_weapon_system.h"
#include "../sdk/c_view_beams.h"
#include "../sdk/c_debug_overlay.h"

void c_damageesp::draw_damage()
{
	auto local = c_cs_player::get_local_player();

	if (!local || !local->is_alive() || !config.esp.hitmarker_damage)
		return;

	float curTime = global_vars_base->curtime;

	for (int i = 0; i < MAX_FLOATING_TEXTS; i++)
	{
		FloatingText* txt = &floatingTexts[i % MAX_FLOATING_TEXTS];

		if (!txt->valid)
			continue;

		float endTime = txt->startTime + 1.1f;

		if (endTime < curTime)
		{
			txt->valid = false;
			continue;
		}

		c_vector2d origin_screen;

		if (!renderer->screen_transform(txt->hitPosition, origin_screen, c_esp::info.world_to_screen_matrix))
			return;

		float t = 1.0f - (endTime - curTime) / (endTime - txt->startTime);

		origin_screen.y -= t * (35.0f);
		origin_screen.x -= (float)txt->randomIdx * t * 3.0f;

		char msg[12];
		sprintf_s(msg, 12, _("-%dHP"), txt->damage);

		uint32_t font = fnv1a("pro13");
		c_vector2d width = renderer->get_text_size(msg, font);

		c_color damage_color;
		if (txt->damage >= 90 && txt->damage <= 800)
			damage_color = c_color(255, 0, 0, 255);
		else if (txt->damage >= 50 && txt->damage <= 89)
			damage_color = c_color(220, 0, 220, 255);
		else if (txt->damage >= 0 && txt->damage <= 49)
			damage_color = c_color(0, 255, 0, 255);

		renderer->text(origin_screen - c_vector2d(0, 18), msg, damage_color, fnv1a("pro13"));
	}
}

void c_damageesp::on_bullet_impact(c_game_event* event)
{
	auto local = c_cs_player::get_local_player();

	if (!local || !local->is_alive() || !config.esp.hitmarker_damage)
		return;

	float x = event->get_float(_("x"));
	float y = event->get_float(_("y"));
	float z = event->get_float(_("z"));

	const auto target = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("userid")))));

	if (!target || target != local)
		return;

	c_impactpos = c_vector3d(x, y, z);

}

void c_damageesp::on_player_hurt(c_game_event* event)
{
	auto local = c_cs_player::get_local_player();

	if (!local || !local->is_alive() || !config.esp.hitmarker_damage)
		return;

	float curTime = global_vars_base->curtime;

	int dmg_health = event->get_int(_("dmg_health"));
	int hitgroup = event->get_int(_("hitgroup"));

	const auto entity = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("userid")))));
	const auto attacker = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("attacker")))));

	if (!entity || attacker != local)
		return;

	FloatingText txt;
	txt.startTime = curTime;
	txt.hitgroup = hitgroup;
	txt.hitPosition = c_impactpos;
	txt.damage = dmg_health;
	txt.randomIdx = math::random_float(-5.f, 5.f);
	txt.valid = true;

	floatingTexts[floatingTextsIdx++ % MAX_FLOATING_TEXTS] = txt;
}