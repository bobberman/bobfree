#include "c_legitbot_beta.h"
#include "../utils/math.h"
#include "../sdk/c_global_vars_base.h"
#include "../sdk/c_cvar.h"
#include "c_animation_system.h"
#include "c_aimhelper.h"
#include "../hooks/idirect3ddevice9.h"


bool c_legitbot_beta::aim(c_user_cmd* cmd){
	if (!config.legit.assist || config.rage.enabled) return false;

	auto local = c_cs_player::get_local_player();
	if (!local || local->get_health() <= 0)return false;
	bool shooting = (cmd->buttons & c_user_cmd::flags::attack) && local->can_shoot(cmd, global_vars_base->curtime, true);
	if (!shooting)return false;
	
	const auto weapon = (c_base_combat_weapon*)client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle());
	if (!weapon)return false;
	/*add a weapon check for knife bomb and shit*/

	if (weapon->is_bomb() || weapon->is_knife() || weapon->is_grenade())return false; // why would we aimbot with that

	std::optional<std::tuple<c_vector3d, float, c_animation_system::animation*, float>> target = std::nullopt;

	if (!target.has_value()){
		std::vector<c_cs_player::hitbox> hitboxes_scan;
		hitboxes_scan.push_back(c_cs_player::hitbox::head); 
		hitboxes_scan.push_back(c_cs_player::hitbox::pelvis);
		hitboxes_scan.push_back(c_cs_player::hitbox::left_foot);
		hitboxes_scan.push_back(c_cs_player::hitbox::right_foot);
		target = c_aimhelper::get_legit_target(config.legit.fov+8, 1, hitboxes_scan);
	}

	if (!target.has_value())return false;

	static bool config_aimbot = true;
	if(config_aimbot){
		static bool config_secondary_fovcheck = true;
		static float config_secondary_fov = 2.5f;
		if (std::get<3>(target.value()) < config_secondary_fov || !config_secondary_fovcheck) {
			const auto view_angle = engine_client()->get_view_angles();
			const auto shoot_position = local->get_shoot_position();
			const auto recoil_scale = cvar()->find_var(_("weapon_recoil_scale"))->get_float();
			
			auto aim_angle = math::calc_angle(shoot_position, std::get<0>(target.value()));

			auto fired_shots = local->get_shots_fired();
			int config_minshots = 1;

			static bool config_beforerecoil = false;
			if (config_beforerecoil) {
				smooth(aim_angle, view_angle);
				
				if (config_minshots < fired_shots)
					aim_angle -= local->get_punch_angle() * recoil_scale;
			}
			else {
				if (config_minshots < fired_shots)
					aim_angle -= local->get_punch_angle() * recoil_scale;
				smooth(aim_angle, view_angle);
			}
			math::normalize(aim_angle);
			cmd->viewangles = aim_angle;
			engine_client()->set_view_angles(aim_angle);
		}
	}
	
	cmd->tick_count = time_to_ticks(std::get<1>(target.value())) + lerp_ticks;
}
void c_legitbot_beta::smooth(c_qangle &angle, c_qangle view_angle) {

	auto delta = angle - view_angle;
	math::normalize(delta);
	
	static float xsmoothing = 4;
	static float ysmoothing = 2;
	delta.x /= xsmoothing;
	delta.y /= ysmoothing;
	angle = view_angle + delta;

	math::normalize(angle);
}