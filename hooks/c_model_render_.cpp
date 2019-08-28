#include "c_model_render_.h"
#include "../utils/c_hook.h"
#include "../sdk/c_model_info_client.h"
#include "../hacks/c_chams.h"
#include "../sdk/c_render_view.h"
#include "../utils/c_config.h"
#include "../sdk/c_cs_player.h"
#include "../sdk/c_input.h"
#include "../sdk/c_client_entity_list.h"
#include "../utils/c_log.h"
#include "../hacks/c_animation_system.h"
#include "../hacks/c_antiaim.h"
#include "../sdk/c_game_rules.h"
#include "../sdk/c_material_system.h"

void c_model_render_::hook()
{
	static c_hook<c_model_render> hook(model_render());
	_draw_model_execute = hook.apply<draw_model_execute_t>(21, draw_model_execute);
}

void c_model_render_::draw_model_execute(c_model_render* model_render, uint32_t, void* ctx, draw_model_state* state, model_render_info_t& info, matrix3x4* matrix)
{
	if (!engine_client()->is_ingame())
		return _draw_model_execute(model_render, ctx, state, info, matrix);

	render_view()->set_color_modulation(c_color(255, 255, 255));

	if (!info.model)
		return _draw_model_execute(model_render, ctx, state, info, matrix);

	

	const auto original = [&]() -> void
	{
		_draw_model_execute(model_render, ctx, state, info, c_chams::instance()->current_matrix ? c_chams::instance()->current_matrix : matrix);
	};

	if (strstr(info.model->name, _("sleeves")) != nullptr && config.chams.hand.option3) {
		auto mat = find_mat(info.model->name);
		mat->set_material_var_flag(material_var_no_draw, true);
		model_render->forced_material_override(mat);
	}else if (strstr(info.model->name, _("arms")) != nullptr)
	{
		if (config.chams.hand.option2) {		
			auto mat = find_mat(info.model->name);
			mat->set_material_var_flag(material_var_no_draw, true);
			model_render->forced_material_override(mat);
		}
		else {
			const auto local = c_cs_player::get_local_player();

			if (!local || !(local->is_alive() || !input->camera_in_third_person || local->is_scoped()))
				return;
			matrix3x4 interpolated[128] = {};

			if (local && config.chams.hand.enabled)
			{

				c_chams::hand_chams(original, config.chams.hand, false);

				_draw_model_execute(model_render, ctx, state, info, interpolated);

				render_view()->set_color_modulation(c_color(255, 255, 255));
				model_render->forced_material_override(nullptr);

			}
			c_chams::hand_chams(original, config.chams.hand, false);
		}
	}
	else if (strstr(info.model->name, _("weapons/v_")) != nullptr)
	{
		const auto local = c_cs_player::get_local_player();

		if (!local || !(local->is_alive() || !input->camera_in_third_person || local->is_scoped()))
			return;
		matrix3x4 interpolated[128] = {};

		if (local && config.chams.weapon.enabled)
		{
			matrix3x4 interpolated[128] = {};

			if (local, interpolated)
			{
	
				c_chams::instance()->current_matrix = interpolated;
				c_chams::weapon_chams(original, config.chams.weapon, false);

				

				_draw_model_execute(model_render, ctx, state, info, interpolated);
				
				render_view()->set_color_modulation(c_color(255, 255, 255));
				model_render->forced_material_override(nullptr);

				c_chams::instance()->current_matrix = nullptr;
			}
		}

		c_chams::weapon_chams(original, config.chams.weapon, false);
	}

	if (strstr(info.model->name, _("models/player")) != nullptr)
	{
		const auto entity = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(info.entity_index));

		if (entity && !entity->is_ragdoll() && entity->is_alive())
		{

			if (!c_chams::instance()->second_pass && c_chams::instance()->current_player == entity && !entity->is_local_player())
				return;

			if (!entity->is_local_player())
			{
				if (entity->is_enemy() && config.chams.backtrack.enabled)
				{
						matrix3x4 interpolated[128] = {};

					if (c_chams::get_simple_backtrack_matrix(entity, interpolated))
					{
						c_chams::instance()->current_matrix = interpolated;
						c_chams::player_chams(original, config.chams.backtrack, false);
						original();
						render_view()->set_color_modulation(c_color(255, 255, 255));
						model_render->forced_material_override(nullptr);
						//c_chams::set_ignorez(false);
						c_chams::instance()->current_matrix = nullptr;
					}
				}

				if (c_chams::instance()->current_matrix)
				{
					c_chams::player_chams(original, config.chams.backtrack, false);
				}
				else if (entity->is_enemy())
				{
					c_chams::player_chams(original, config.chams.enemy, false);
				}
			}
			else if (entity->is_local_player())
			{
				if (!game_rules->is_freeze_period())
				{
					if (antiaim->last_fake_matrix && config.chams.local.desync_type > 0)
					{
						c_chams::instance()->current_matrix = antiaim->last_fake_matrix;

						if (c_chams::instance()->current_matrix)
							c_chams::player_chams(original, config.chams.local, true, (entity->is_local_player() && entity->is_scoped() && input->camera_in_third_person));

						original();
						render_view()->set_color_modulation(c_color(255, 255, 255));
						model_render->forced_material_override(nullptr);
						//c_chams::set_ignorez(false);
						c_chams::instance()->current_matrix = nullptr;
					}
				}
				c_chams::player_chams(original, config.chams.local, false, (entity->is_local_player() && entity->is_scoped() && config.chams.local.scope_blend && input->camera_in_third_person));
			}
		}
	}


	original();
	_draw_model_execute(model_render, ctx, state, info, c_chams::instance()->current_matrix ? c_chams::instance()->current_matrix : matrix);
	model_render->forced_material_override(NULL);
}
