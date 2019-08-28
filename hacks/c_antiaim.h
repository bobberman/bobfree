#pragma once

#include "../includes.h"

class c_antiaim : public c_singleton<c_antiaim>
{
public:
	static int GetDirection();
	void fakelag(c_cs_player* local, c_user_cmd* cmd, bool& send_packet);
	void run(c_cs_player* local, c_user_cmd* cmd, bool& send_packet, int sequence);
	void prepare_animation(c_cs_player* local);
	void predict(c_cs_player* local, c_user_cmd* cmd);
	void shooting(c_cs_player* local, c_user_cmd* cmd, float ideal_yaw, int direction, bool send_packet);

	float get_pitch(c_cs_player* local, c_user_cmd* cmd);
	float get_yaw(c_cs_player* local, c_user_cmd* cmd, float ideal_yaw, int direction);

	void increment_visual_progress();

	float get_visual_choke();
	float get_last_real();
	float get_last_fake();

	uint32_t shot_cmd{};

	matrix3x4 last_fake_matrix[128] = { matrix3x4() };
	matrix3x4 last_real_matrix[128] = { matrix3x4() };
	
	float last_real = 0.f, last_fake = 0.f;

	bool is_fakeducking = false;
	bool is_slow_walking = false;
	bool is_on_peek = false;
	bool is_lby_broken = false;

	float visual_choke = 0.f,
		next_lby_update = 0.f, lby_update = 0.f,
		min_delta = 0.f, max_delta = 0.f,
		stop_to_full_running_fraction = 0.f,
		feet_speed_stand = 0.f, feet_speed_ducked = 0.f;

	uint32_t estimated_choke = 0;

private:
	bool on_peek(c_cs_player* local, bool& target);
	float calculate_ideal_yaw(c_cs_player* local, bool estimate = false);

	bool is_standing = false;
};

#define antiaim c_antiaim::instance()
