#pragma once

#include "../includes.h"
#include "c_animation_system.h"
#include "c_aimhelper.h"


class c_aimbot : public c_singleton<c_aimbot>
{
	struct autostop_info
	{
		float call_time;
		bool did_stop;
	};

public:
	bool is_autoshooting = false;

	static void aimbot_run(c_cs_player* local, c_user_cmd* cmd);
	static bool aimbot_target(c_cs_player* target, c_user_cmd* cmd, c_cs_player* local);
	static c_vector3d aimbot_get_bone(c_cs_player *player, c_cs_player* local, c_cs_player::hitbox prioritized, float minDmg, bool onlyPrioritized, c_animation_system::animation* animation);
	static float aimbot_best_point(c_cs_player *player, c_cs_player* local, c_cs_player::hitbox prioritized, float minDmg, mstudiohitboxset_t *hitset, c_vector3d &vecOut, c_animation_system::animation* animation);

	inline static std::optional<float> last_pitch = std::nullopt;
private:
	
};

#define aimbot c_aimbot::instance()