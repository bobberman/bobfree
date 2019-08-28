#pragma once

#include "../includes.h"
#include "../hooks/idirect3ddevice9.h"
#include "../sdk/c_game_event_manager.h"

#include <mutex>

#define MAX_FLOATING_TEXTS 50

class FloatingText
{
public:
	bool valid = false;
	float startTime = 1.f;
	int damage = 0;
	int hitgroup = 0;
	c_vector3d hitPosition = c_vector3d(0, 0, 0);
	int randomIdx = 0;
};

class c_damageesp : public c_singleton<c_damageesp>
{
public:
	void draw_damage();
	void on_player_hurt(c_game_event* event);
	void on_bullet_impact(c_game_event* event);
private:
	std::array<FloatingText, MAX_FLOATING_TEXTS> floatingTexts;
	c_vector3d c_impactpos = c_vector3d(0, 0, 0);
	int floatingTextsIdx = 0;
};
#define damageesp c_damageesp::instance()