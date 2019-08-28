#pragma once

#include <deque>
#include <utility>
#include <mutex>
#include "../sdk/c_cs_player.h"
#include "../sdk/c_client_state.h"
#include "../sdk/c_global_vars_base.h"
#include "../sdk/c_cvar.h"
#include "../utils/math.h"

// sse max.
template< typename t = float >
t c_maxes(const t &a, const t &b) {
	// check type.
	static_assert(std::is_arithmetic< t >::value, "math::max only supports integral types.");
	return (t)_mm_cvtss_f32(
		_mm_max_ss(_mm_set_ss((float)a),
			_mm_set_ss((float)b))
	);
}

enum backtrack_mode {
	backtrack_high,
	backtrack_last_record
};

enum resolver_state /*WHEN STANDING/SLOW WALKING USE THIS FOR NOW*/
{
	resolver_start,
	resolver_inverse,
	resolver_no_desync,
	resolver_jitter
};

enum resolver_state_moving { /* LETS ATTEMPT A DIFFERENT METHOD OF MOVING RESOLVER*/
	resolver_moving_start,
	resolver_moving_start_inverse,
	resolver_moving_inverse,
	resolver_moving_no_desync,
	resolver_moving_jitter,
};

enum resolver_slow_walk_state {
	resolver_slow_walk_begin,
	resolver_slow_walk_inverse
};

__forceinline float calculate_lerp()
{
	static auto cl_ud_rate = cvar()->find_var("cl_updaterate");
	static auto min_ud_rate = cvar()->find_var("sv_minupdaterate");
	static auto max_ud_rate = cvar()->find_var("sv_maxupdaterate");

	int ud_rate = 64;

	if (cl_ud_rate)
		ud_rate = cl_ud_rate->get_int();

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->get_int();

	float ratio = 1.f;
	static auto cl_interp_ratio = cvar()->find_var("cl_interp_ratio");
	
	if (cl_interp_ratio)
		ratio = cl_interp_ratio->get_float();

	static auto cl_interp = cvar()->find_var("cl_interp");
	static auto c_min_ratio = cvar()->find_var("sv_client_min_interp_ratio");
	static auto c_max_ratio = cvar()->find_var("sv_client_max_interp_ratio");

	float lerp = global_vars_base->interval_per_tick;

	if (cl_interp)
		lerp = cl_interp->get_float();

	if (c_min_ratio && c_max_ratio && c_min_ratio->get_float() != 1)
		ratio = std::clamp(ratio, c_min_ratio->get_float(), c_max_ratio->get_float());

	return c_maxes(lerp, ratio / ud_rate);
}

class c_animation_system : public c_singleton<c_animation_system>
{
public:
	struct animation
	{
		animation() = default;

		explicit animation(c_cs_player* player);
		explicit animation(c_cs_player* player, c_qangle last_reliable_angle);

		void restore(c_cs_player* player) const;
		void apply(c_cs_player* player) const;
		void build_server_bones(c_cs_player* player);

		bool is_valid(float sim_time, bool this_valid, const float range = .2f);

		c_cs_player* player{};
		int32_t index{};

		bool valid{}, has_anim_state{};
		alignas(16) matrix3x4 bones[128]{};

		bool dormant{};

		c_vector3d velocity;
		c_vector3d origin;
		c_vector3d abs_origin;
		c_vector3d obb_mins;
		c_vector3d obb_maxs;

		c_base_animating::animation_layers layers{};
		c_base_animating::pose_paramater poses{};

		c_csgo_player_anim_state anim_state{};

		float anim_time{};
		float sim_time{};
		float interp_time{};
		float duck{};
		float lby{};
		float last_shot_time{};

		c_qangle last_reliable_angle{};
		c_qangle eye_angles;
		c_qangle abs_ang;

		int flags{};
		int eflags{};
		int effects{};
		int lag{};
		int didshot{};
		int sideways{};
		int upPitch{};
		int record_priority{};
	};
private:
	struct animation_info {
		animation_info(c_cs_player* player, std::deque<animation> animations)
			: player(player), frames(std::move(animations)), last_spawn_time(0) { }

		void update_animations(animation* to, animation* from);

		c_cs_player* player{};
		std::deque<animation> frames{};

		// latest animation (might be invalid)
		animation latest_animation{};

		// last time this player spawned
		float last_spawn_time;

		// counter of how many shots we missed
		int32_t missed_due_to_spread{};
		int32_t missed_due_to_resolver{};

		// resolver data
		resolver_state brute_state{};
		resolver_state_moving brute_moving_state{};
		resolver_slow_walk_state brute_slowwalk_state{};

		float brute_yaw{};
		float moving_brute_yaw{};
		float slowwalk_brute_yaw{};

		c_vector3d last_reliable_angle{};
	};

	std::unordered_map<c_base_handle, animation_info> animation_infos;

public:
	void update_player(c_cs_player* player);
	void update_simple_local_player(c_cs_player* player, c_user_cmd* cmd);
	void post_player_update();

	animation_info* get_animation_info(c_cs_player* player);

	//
	std::optional<animation*> get_latest_animation(c_cs_player* player);
	std::optional<animation*> get_oldest_animation(c_cs_player* player);
	std::optional<animation*> get_uncrouched_animation(c_cs_player* player);
	std::optional<animation*> get_latest_upPitch_animation(c_cs_player* player);//new
	std::optional<animation*> get_latest_sideways_animation(c_cs_player* player);//new

	std::optional<animation*> get_latest_firing_animation(c_cs_player* player);
	std::optional<animation*> get_oldest_firing_animation(c_cs_player* player);
	std::optional<animation*> get_firing_uncrouched_animation(c_cs_player* player);
	//

	std::optional<std::pair<animation*, animation*>> get_intermediate_animations(c_cs_player* player, float range = 1.f);
	std::optional<animation*> get_lastest_animation_unsafe(c_cs_player* player);

	std::vector<animation*> get_valid_animations(c_cs_player* player, float range = 1.f);

	animation local_animation;
	c_base_animating::animation_layers server_layers{};
	bool in_jump{}, enable_bones{};
	c_csgo_player_anim_state* last_process_state{};
};

#define animation_system c_animation_system::instance()
#define lerp_ticks time_to_ticks(calculate_lerp())