#pragma once

#include <functional>
#include "../utils/c_config.h"
#include "../sdk/c_cs_player.h"

class c_chams : public c_singleton<c_chams>
{
public:
	c_chams();

	void latch_timer();

	static void draw_players();
	static bool get_backtrack_matrix(c_cs_player* player, matrix3x4* out);
	static bool get_simple_backtrack_matrix(c_cs_player* player, matrix3x4* out);
	static c_material * GetMat(int material);

	static c_material * GetOverlay(int material);

	

	static void CustomMaterial(c_material * mat);

	static void player_chams(std::function<void()> original, c_config::config_conf::chams_conf::chams& conf, bool draw_fake, bool scope_blend = false);
	static void hand_chams(std::function<void()> original, c_config::config_conf::chams_conf::chams& conf, bool draw_hand);
	static void weapon_chams(std::function<void()> original, c_config::config_conf::chams_conf::chams& conf, bool draw_weapon);
	static void set_ignorez(bool enabled, c_material* mat);
	

	c_cs_player* current_player;
	matrix3x4* current_matrix;
	bool second_pass;

private:
	static void modulate(c_color color, c_material* material = nullptr);
	static void modulate_exp(c_material* material, float alpha = 1.f, float width = 6.f);
	static void modulate_reflectivity(c_material* material);
	int alpha;
	bool direction;
};
