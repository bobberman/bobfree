#pragma once

#include "../sdk/c_game_event_manager.h"

class c_hitmarker
{
public:
	static void draw();
	static void on_player_hurt(c_game_event* event);
	static void deadsound(c_game_event* event);
	static void on_bullet_impact(c_game_event* event);
	static void on_round_start(c_game_event* event);
	//inline static matrix3x4 *setup_bones_matrix[64];//index/matrix
private:
};

struct impact_info {
	float x, y, z;
	float time;
};

struct hitmarker_info {
	impact_info impact;
	int alpha;
};

extern std::vector<impact_info> impacts;
extern std::vector<hitmarker_info> hitmarkers;
