#include "../BASS/API.h"

#include "c_aimhelper.h"
#include "c_hitmarker.h"
#include "c_esp.h"
#include "c_trace_system.h"

#include "../sdk/c_debug_overlay.h"
#include "../sdk/c_client_entity_list.h"
#include "../hooks/idirect3ddevice9.h"
#include "../hacks/c_resolver.h"

std::vector<impact_info> impacts;
std::vector<hitmarker_info> hitmarkers;

void c_hitmarker::draw()
{
	float time = global_vars_base->curtime;

	for (int i = 0; i < hitmarkers.size(); i++) 
	{
		bool expired = time >= hitmarkers.at(i).impact.time + 1.f;

		if (expired)
			hitmarkers.at(i).alpha -= 1;

		if (expired && hitmarkers.at(i).alpha <= 0) {
			hitmarkers.erase(hitmarkers.begin() + i);
			continue;
		}

		c_vector3d pos3D = c_vector3d(hitmarkers.at(i).impact.x, hitmarkers.at(i).impact.y, hitmarkers.at(i).impact.z);
		c_vector2d pos2D;

		if (!renderer->screen_transform(pos3D, pos2D, c_esp::info.world_to_screen_matrix))
			continue;

		auto linesize = 8;



		renderer->line(c_vector2d(pos2D.x - linesize, pos2D.y - linesize), c_vector2d(pos2D.x - (linesize / 4), pos2D.y - (linesize / 4)), c_color(config.esp.hitmarker_color));
		renderer->line(c_vector2d(pos2D.x - linesize, pos2D.y + linesize), c_vector2d(pos2D.x - (linesize / 4), pos2D.y + (linesize / 4)), c_color(config.esp.hitmarker_color));
		renderer->line(c_vector2d(pos2D.x + linesize, pos2D.y + linesize), c_vector2d(pos2D.x + (linesize / 4), pos2D.y + (linesize / 4)), c_color(config.esp.hitmarker_color));
		renderer->line(c_vector2d(pos2D.x + linesize, pos2D.y - linesize), c_vector2d(pos2D.x + (linesize / 4), pos2D.y - (linesize / 4)), c_color(config.esp.hitmarker_color));
	}

}

void c_hitmarker::on_round_start(c_game_event* event)
{
	if (!config.esp.hitmarker)
		return;

	if (hitmarkers.size() > 0)
		hitmarkers.clear();
}

void c_hitmarker::on_bullet_impact(c_game_event* event)
{
	if (!config.esp.hitmarker)
		return;

	auto local = c_cs_player::get_local_player();

	if (!local || !local->is_alive())
		return;

	impact_info info;

	info.x = event->get_float("x");
	info.y = event->get_float("y");
	info.z = event->get_float("z");

	info.time = global_vars_base->curtime;

	impacts.push_back(info);

}

void c_hitmarker::deadsound(c_game_event* event)
{
	

}

void c_hitmarker::on_player_hurt(c_game_event* event)
{
	auto local = c_cs_player::get_local_player();


	const auto attacker = client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("attacker"))));
	const auto target = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_player_for_user_id(event->get_int(_("userid")))));

	if (!local || !local->is_alive())
		return;

	if (attacker && target && attacker == local && target->is_enemy())
	{

		if (config.misc.hit_effect && target->is_alive())
			local->m_flHealthShotBoostExpirationTime() = global_vars_base->curtime + 0.5f;

		if (config.misc.kill_effect && !target->is_alive())
			local->m_flHealthShotBoostExpirationTime() = global_vars_base->curtime + 1.f;
	
		uint32_t hitsound = 0;
		if(config.esp.hitsound)
			hitsound = BASS::stream_sounds.cod_hitsound;
		

		if (hitsound)
		{
			BASS_SET_VOLUME(hitsound, config.esp.hitsound_volume / 100.f);
			BASS_ChannelPlay(hitsound, true);
		}

		if (config.esp.hitmarker)
		{
			impact_info best_impact;

			float best_impact_distance = -1;

			float time = global_vars_base->curtime;

			for (int i = 0; i < impacts.size(); i++)
			{
				auto iter = impacts[i];

				if (time > iter.time + 1.f)
				{
					impacts.erase(impacts.begin() + i);
					continue;
				}

				c_vector3d position = c_vector3d(iter.x, iter.y, iter.z);

				c_vector3d enemy_pos = target->get_origin();

				float distance = position.dist_to(enemy_pos);

				if (distance < best_impact_distance || best_impact_distance == -1)
				{
					best_impact_distance = distance;
					best_impact = iter;
				}
			}

			if (best_impact_distance == -1)
				return;

			hitmarker_info info;

			info.impact = best_impact;
			info.alpha = 255;

			hitmarkers.push_back(info);
		}
		if (config.esp.show_lag_compensation)
		{
			matrix3x4 animation_matrix[128];
			matrix3x4 setup_bones_matrix[128];

			auto animations = animation_system->get_latest_animation(target);

			if (target->setup_bones(setup_bones_matrix, 128, bone_used_by_anything, target->get_simtime()))
			{
				const auto model = target->get_model();

				if (!model)
					return;

				const auto hdr = model_info_client()->get_studio_model(model);

				if (!hdr)
					return;
			
				if (!animations.has_value())
					return;
			
				std::memcpy(animation_matrix, animations.value()->bones, sizeof(animations.value()->bones));
			//	std::memcpy(animation_matrix, c_hitmarker::setup_bones_matrix[event->get_int(_("userid"))], sizeof(c_hitmarker::setup_bones_matrix[event->get_int(_("userid"))]));

				const auto set = hdr->get_hitbox_set(0);

				if (set)
				{
					for (auto i = 0; i < set->numhitboxes; i++)
					{
						const auto hitbox = set->get_hitbox(i);

						if (!hitbox)
							continue;
						int r = config.esp.show_lag_compensation_color.red;
						int g = config.esp.show_lag_compensation_color.green;
						int b = config.esp.show_lag_compensation_color.blue;
						int a = config.esp.show_lag_compensation_color.alpha;
						if (hitbox->radius == -1.0f)
						{
							const auto position = math::matrix_position(animation_matrix[hitbox->bone]);
							const auto position_actual = math::matrix_position(setup_bones_matrix[hitbox->bone]);

							const auto roation = math::angle_matrix(hitbox->rotation);

							auto transform = math::multiply_matrix(animation_matrix[hitbox->bone], roation);
							auto transform_actual = math::multiply_matrix(setup_bones_matrix[hitbox->bone], roation);

							const auto angles = math::matrix_angles(transform);
							const auto angles_actual = math::matrix_angles(transform_actual);

							//debug_overlay()->add_box_overlay(position, hitbox->bbmin, hitbox->bbmax, angles, 255, 0, 0, 150, 0.8f);
							debug_overlay()->add_box_overlay(position_actual, hitbox->bbmin, hitbox->bbmax, angles_actual, r, g, b, a, 0.8f);
						}
						else
						{
							c_vector3d min, max, min_actual, max_actual;

							math::vector_transform(hitbox->bbmin, animation_matrix[hitbox->bone], min);
							math::vector_transform(hitbox->bbmax, animation_matrix[hitbox->bone], max);

							math::vector_transform(hitbox->bbmin, setup_bones_matrix[hitbox->bone], min_actual);
							math::vector_transform(hitbox->bbmax, setup_bones_matrix[hitbox->bone], max_actual);

							//debug_overlay()->add_capsule_overlay(min, max, hitbox->radius, 255, 0, 0, 150, 0.8f);
							debug_overlay()->add_capsule_overlay(min_actual, max_actual, hitbox->radius, r, g, b, a, 0.8f);
						}
					}
				}
			}
		}
	}
}
