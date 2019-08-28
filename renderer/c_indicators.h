#pragma once

#include "../sdk/c_vector2d.h"
#include "../sdk/c_cs_player.h"

class c_color;

class c_indicators
{
public:
	static void draw();

private:
	static void draw_radio_info(c_vector2d& position);
	static void draw_ping_acceptance(c_vector2d& position);
	static void draw_choke(c_vector2d& position);

	static void draw_antiaim(const char* text, float angle, c_color color, c_cs_player* local);

	static void draw_progressbar(c_vector2d& position, const char* name, c_color color, float progress);
	static void draw_watermark(c_vector2d& position, const char* name);
	static void draw_arrow(float angle, c_color color);
};
