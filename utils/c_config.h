#pragma once

#include "c_singleton.h"
#include "../security/fnv1a.h"
#include "../security/string_obfuscation.h"
#include "../sdk/c_color.h"
#include "../sdk/c_cvar.h"
#include "base64.h"
#include <fstream>

using json = nlohmann::json;

#define make_key(var) std::to_string(fnv1a(var))

class c_config : public c_singleton<c_config>
{
public:
	struct config_conf
	{
		struct esp_conf
		{
			struct player
			{
				bool box, name, impacts, skeleton, history_skeleton, radar, health, ammo, weapon, indicators, flags, head_multipoint, Aspectratio;
				float Aspectratio_slider;
			};



			struct flags {
				bool zoom, reload, defuse, resolver, kevlar;
			};

			player enemy;
			flags enemy_flags;

			bool nade_esp;
			c_color color, impacts_color, nade_color, ammo_color, name_color, box_color, skeleton_color, local_impacts_color, radar_color, weapon_color;

			bool hitmarker;
			c_color hitmarker_color;
			bool hitsound;
			float hitsound_volume;
			

			
			bool local_impact;
			bool show_lag_compensation;
			c_color show_lag_compensation_color;
			bool show_on_shot_hitboxes;
			bool grenade_pred;
			c_color grenade_predc; int grenade_preds;
			float local_impact_width;
			bool hitmarker_damage;
			bool multipoint;

			// styling
			bool health_based; c_color health_color; int health_dividers;
			bool armor; c_color armor_color;
			bool wep_icons;
		};



		struct chams_conf
		{
			struct chams
			{

				int type;
				int desync_type;
				int desync_type2;
				c_color color;
				c_color desync_color;
				c_color xqz_color;
				bool xqz;
				bool enabled;
				bool scope_blend;
				float alpha;
				float dalpha;

				bool option1;
				bool option2;
				bool option3;
				bool option4;

			};

			chams enemy, backtrack, local, hand, weapon;
			int base;
			bool show_desync;
			bool option1;
			bool option2;
			bool option3;
		};

		struct legit_conf
		{
			bool assist;
			float fov, smooth, backtrack;
			bool only_backtrack;
		};

		struct rage_conf
		{
			int resolver;

			struct weapon_conf
			{
				float hitchance_body, hitchance_head, min_dmg, head_scale, chest_scale, stomach_scale, pelvis_scale, legs_scale, feet_scale, optimize_fps;
				int body_aim_health, body_aim_key, head_aim_key, body_after_x_missed_spread, body_after_x_missed_resolver;
				bool hitscan_head, hitscan_chest, hitscan_stomach, hitscan_pelvis, hitscan_legs, hitscan_feet, on_shot_hitscan_head, on_shot_hitscan_body;
				bool hitscan_head_moving, hitscan_chest_moving, hitscan_stomach_moving, hitscan_pelvis_moving, hitscan_legs_moving, hitscan_feet_moving;
				bool auto_shoot, head_aim_only_while_firing, smart_aim, override_head_aim_only;
				bool body_aim_in_air, body_aim_lethal, body_aim_slow_walk, body_aim_if_not_on_shot, min_dmg_hp;
				int min_dmg_hp_slider;
				float hp, hp_health_override, hp_head_hitchance, hp_body_hitchance, hp_mindmg;


			};

			struct antiaim_conf {
				bool enabled;

				int pitch_mode, yaw_mode;
				float yaw_add, custom_pitch, jitter_range, jitter_speed;
				/*MOVING ANIT AIM BELOW*/
				int pitch_moving_mode, yaw_moving_mode;
				float yaw_moving_add, custom_moving_pitch, jitter_moving_range, jitter_moving_speed;
				/*air ANIT AIM BELOW*/
				int pitch_air_mode, yaw_air_mode;
				float yaw_air_add, custom_air_pitch,  jitter_air_range, jitter_air_speed;
				/*slow ANIT AIM BELOW*/
				int pitch_slow_mode, yaw_slow_mode;
				float yaw_slow_add, custom_slow_pitch, jitter_slow_range, jitter_slow_speed;

			};

			struct fakelag_conf
			{
				bool enabled;
				int fake_lag_standing, fake_lag_moving, fake_lag_air, fake_lag_slowwalk;
				bool fake_lag_on_peek_delay, disable_on_knife, disable_on_tazer, disable_on_grenade, fake_lag_automatic, disable_on_shooting, disable_on_revolver;

			};

			bool health, headonly;
			bool enabled, slide, adaptive_history_backtrack, experiemental_resolver;
			int slow_walk, fake_duck, backtrack_mode, auto_stop;


			antiaim_conf antiaim_settings;
			fakelag_conf fakelag_settings;

			weapon_conf auto_snipe, scout, awp, pistol, heavy, misc;
		};

		struct misc_conf
		{
			int inverter;
			int flip;
			//int direction;
			bool  gravity, knife;
			bool on_screenplayers;
			int on_screen_x = 500;
			int on_screen_y = 500;
			int on_ind_x = 5;
			int on_ind_y = 500;

			float slow_walk_speed;

			int radio_channel;
			float radio_volume;
			int radio_mute;

			float spike_ping;
			bool ping_switch;

			float thirdperson_dist;
			int thirdperson_switch;

			bool buy_bot;
			int buy_bot_primary, buy_bot_secondary, buy_bot_armor;
			bool buy_bot_grenades, buy_bot_zeus, buy_bot_defuser;

			bool engine_radar, no_recoil, no_smoke, no_flash, no_scope, no_draw, no_post_processing, no_fog, remove_zoom, remove_punch, nightmode, indicators, arrows, full_bright, sv_impacts, preserve_feed;

			int nightmode_selection;
			c_color nightmode_color;
			float nightmode_darkness;

			float fov, fov_view;
			float transparency;
			bool clantag;

			int hotkey_x;
			int hotkey_y;
			bool hotkeyss;
			bool watermark;
			c_color theme;

			bool indicator_antiaim;
			bool indicator_antiaim_l;
			c_color indicator_antiaimc;

			bool streamermode;
			bool hit_effect;
			bool kill_effect;
		};

		esp_conf esp;
		chams_conf chams;
		legit_conf legit;
		rage_conf rage;
		misc_conf misc;
		float version_number;
	};

	c_config() = default;

	config_conf& get()
	{
		static config_conf config
		{
			// esp
			{
				// esp.enemy
				{
					false,
					false,
					false,
					false,
					false,
					false,
					false,
					false,
					false,
					false,
					false,
					false,
					false,
				},

				// esp.flags.enemy
				{
					false,
					false,
					false,
					false,
					false,
					
				},

			
				false,
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
				c_color(255, 0, 0, 255),
			
				false,
				c_color(255, 0, 0, 255),
				false,
				100.f,
				false,
				true,
				
				
				c_color(255, 0, 0, 255),
				false,
				false,
				c_color(255, 0, 0, 255),
				0,
				0.f,
				false,
				false,
				false,
				c_color(255, 0, 0, 255),
				0,
				false,
				c_color(255,0,0,255),
				false,
			},
			// chams
		{


			// chams.enemy
		{
			0,
			0,
			0,
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			false,
			false,
			false,
			100.f,
			false,
			false,
			false,
			false,
		},
		// chams.backtrack
		{
			0,
			0,
			0,
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			false,
			false,
			false,
			100.f,
			100.f,
			false,
			false,
			false,
			false,

		},
		// chams.local
		{
			0,
			0,
			0,
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			false,
			false,
			false,
			100.f,
			100.f,
			false,
			false,
			false,
			false,

		},

		//hand
		{
			0,
			0,
			0,
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			false,
			false,
			false,
			100.f,
			100.f,
			false,
			false,
			false,
			false,
		},

		//weapon
		{
			0,
			0,
			0,
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			c_color(255, 0, 0, 255),
			false,
			false,
			false,
			100.f,
			100.f,
			false,
			false,
			false,
			false,
		},




		//reflectivity	
		0,
		false,
		false,
		false,
		false,

},
// legit
{
	false,
	0.f,
	0.f,
	0.f,
	false
},
// rage
{
	0,
	false,
	false,
	false,
	false,
	0,
	0,
	0,
	0,
	false,
	false,

	//antiaim
	{

		false,
		0,
		0,
		0.f,
		
		0.f,
		0.f,
		0.f,
		/*MOVING*/
		0,
		0,
		
		0.f,
		0.f,
		0.f,
		0.f,
		/*air*/
		0,
		0,
		
		0.f,
		0.f,
		0.f,
		0.f,
		/*slow*/
		0,
		0,
		0.f,
		
		0.f,
		0.f,
		0.f,
	},

	//fakelag
	{
		false,
		0,
		0,
		0,
		0,
		false,
		false,
		false,
		false,
		false,
		false
	},

			// rage.auto_snipe
			{


				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				1,
			},
			// rage.scout
			{


				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

				0,
				0,
				0,
				0,
				0,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				1,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
			},
			// rage.awp
			{


				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

				0,
				0,
				0,
				0,
				0,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				1,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
			},
			// rage.pistol
			{


				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

				0,
				0,
				0,
				0,
				0,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				1,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

			},
			// rage.heavy
			{


				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

				0,
				0,
				0,
				0,
				0,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				1,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

			},
			// rage.misc
			{


				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,

				0,
				0,
				0,
				0,
				0,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				0,
				0.f,
				0.f,
				0.f,
				0.f,
				0.f,
			}
		},
			// misc
			{
				0,
				0,
				false,	false,
				false, 500, 500,
				5, 500,

				100.f,

				0,
				0.f,
				0,

				0.f,
				0,

				0.f,
				0,

				false,
				0, 0, 0,
				false, false, false,

				false,	false,	false,	false,	false,	false,  false,  false,	false,	false,	false,	false,	false, false, false,
				false,
				0,
				c_color(255, 0, 0, 255),
				50.f,

				50.f,
				50.f,
				100.f,
				false,

				100,
				100,
				false,
				true,
				c_color(255, 0, 0, 255),
				false,
				false,
				c_color(255, 0, 0, 255),
				false,
				false,
				false,
			},
			1.2f
		};

		return config;
	}

	static void load_from_file(const std::string& config_path);
	static void save_to_file(const std::string& config_path);
};

inline void to_json(nlohmann::json& j, const c_config::config_conf::esp_conf::player& value)
{
	j = json{
		{ _("name"), value.name },
		{ _("box"), value.box },
		{ _("impacts"), value.impacts },
		{ _("radar"), value.radar },
		{ _("ammo"), value.ammo },
		{ _("weapon"), value.weapon },
		{ _("indicators"), value.indicators },
		{ _("flags"), value.flags },
		{ _("head_multipoint"), value.head_multipoint },
		{ _("health"), value.health },
		{ _("skeleton"), value.skeleton },
		{ _("history_skeleton"), value.history_skeleton },
		{ _("Aspectratio"), value.Aspectratio },
		{ _("Aspectratio_slider"), value.Aspectratio_slider },
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::esp_conf::player& value)
{
	value.name = j.value(_("name"), false);
	value.box = j.value(_("box"), false);
	value.impacts = j.value(_("impacts"), false);
	value.radar = j.value(_("radar"), false);
	value.ammo = j.value(_("ammo"), false);
	value.weapon = j.value(_("weapon"), false);
	value.indicators = j.value(_("indicators"), false);
	value.flags = j.value(_("flags"), false);
	value.head_multipoint = j.value(_("head_multipoint"), false);
	value.health = j.value(_("health"), false);
	value.skeleton = j.value(_("skeleton"), false);
	value.history_skeleton = j.value(_("history_skeleton"), false);
	value.Aspectratio = j.value(_("Aspectratio"), false);
	value.Aspectratio_slider = j.value(_("Aspectratio_slider"), 0.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::esp_conf::flags& value)
{
	j = json{
		
		{ _("zoom"), value.zoom },
		{ _("defuse"), value.defuse },
		{ _("kevlar"), value.kevlar },
		{ _("reload"), value.reload },
		{ _("resolver"), value.resolver },

	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::esp_conf::flags& value)
{

	value.zoom = j.value(_("zoom"), false);
	value.defuse = j.value(_("defuse"), false);
	value.kevlar = j.value(_("kevlar"), false);
	value.reload = j.value(_("reload"), false);
	value.resolver = j.value(_("resolver"), false);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::esp_conf& value)
{
	j = json{
		
		{ _("enemy"), value.enemy },
		{ _("enemy_flags"), value.enemy_flags },
		{ _("nade_esp"), value.nade_esp },
		{ _("color"), value.color },
		{ _("impacts_color"), value.impacts_color },
		{ _("nade_color"), value.nade_color },
		{ _("ammo_color"), value.ammo_color },
		{ _("name_color"), value.ammo_color },
		{ _("box_color"), value.box_color },
		{ _("skeleton_color"), value.skeleton_color },
		{ _("local_impacts_color"), value.local_impacts_color },
		{ _("radar_color"), value.radar_color },
		{ _("weapon_color"), value.weapon_color },
		{ _("hitmarker"), value.hitmarker },
		{ _("hitmarker_color"), value.hitmarker_color },
		{ _("hitsound"), value.hitsound },
		{ _("hitsound_volume"), value.hitsound_volume },
		
		{ _("local_impact"), value.local_impact },
		{ _("show_lag_compensation"), value.show_lag_compensation },
		{ _("show_lag_compensation_color"), value.show_lag_compensation_color },
		{ _("show_on_shot_hitboxes"), value.show_on_shot_hitboxes },
		{ _("grenade_pred"), value.grenade_pred },
		{ _("grenade_predc"), value.grenade_predc },
		{ _("grenade_preds"), value.grenade_preds },
		{ _("local_impact_width"), value.local_impact_width },
		{ _("hitmarker_damage"), value.hitmarker_damage },
		{ _("multipoint"), value.multipoint },
		{ _("health_based"), value.health_based },
		{ _("armor"), value.armor },
		
		{ _("health_color"), value.health_color },
		{ _("armor_color"), value.armor_color },
		
		{ _("wep_icons"), value.wep_icons }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::esp_conf& value)
{
	
	value.enemy = j.value(_("enemy"), c_config::config_conf::esp_conf::player{});
	value.enemy_flags = j.value(_("enemy_flags"), c_config::config_conf::esp_conf::flags{});
	value.nade_esp = j.value(_("nade_esp"), false);
	value.color = j.value(_("color"), c_color(255, 0, 0, 255));
	value.impacts_color = j.value(_("impacts_color"), c_color(255, 0, 0, 255));
	value.nade_color = j.value(_("nade_color"), c_color(255, 0, 0, 255));
	value.ammo_color = j.value(_("ammo_color"), c_color(255, 0, 0, 255));
	value.name_color = j.value(_("name_color"), c_color(255, 0, 0, 255));
	value.box_color = j.value(_("box_color"), c_color(255, 0, 0, 255));
	value.skeleton_color = j.value(_("skeleton_color"), c_color(255, 0, 0, 255));
	value.local_impacts_color = j.value(_("local_impacts_color"), c_color(255, 0, 0, 255));
	value.radar_color = j.value(_("radar_color"), c_color(255, 0, 0, 255));
	value.weapon_color = j.value(_("weapon_color"), c_color(255, 0, 0, 255));
	value.hitmarker = j.value(_("hitmarker"), false);
	value.hitmarker_color = j.value(_("hitmarker_color"), c_color(255, 0, 0, 255));
	value.hitsound = j.value(_("hitsound"), false);
	value.hitsound_volume = j.value(_("hitsound_volume"), 100.f);
	
	value.local_impact = j.value(_("local_impact"), false);
	value.show_lag_compensation = j.value(_("show_lag_compensation"), false);
	value.show_lag_compensation_color = j.value(_("show_lag_compensation_color"), c_color(255, 0, 0, 255));
	value.show_on_shot_hitboxes = j.value(_("show_on_shot_hitboxes"), false);
	value.grenade_pred = j.value(_("grenade_pred"), false);
	value.grenade_predc = j.value(_("grenade_predc"), c_color(255, 0, 0, 255));
	value.grenade_preds = j.value(_("grenade_preds"), 0);
	value.local_impact_width = j.value(_("local_impact_width"), 0.f);
	value.hitmarker_damage = j.value(_("hitmarker_damage"), false);
	value.multipoint = j.value(_("multipoint"), false);
	value.health_based = j.value(_("health_based"), false);
	value.armor = j.value(_("armor"), false);
	value.health_color = j.value(_("health_color"), c_color(255, 0, 0, 255));

	value.armor_color = j.value(_("armor_color"), c_color(255, 0, 0, 255));
	
	value.wep_icons = j.value(_("wep_icons"), false);
	
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::chams_conf::chams& value)
{
	j = json{
		{ _("type"), value.type },
		{ _("desync_type"), value.desync_type },
		{ _("desync_type2"), value.desync_type2 },
		{ _("color"), value.color },
		{ _("desync_color"), value.desync_color },
		{ _("xqz_color"), value.xqz_color },
		{ _("xqz"), value.xqz },
		{ _("enabled"), value.enabled },
		{ _("scope_blend"), value.scope_blend },
		{ _("alpha"), value.alpha },
		{ _("dalpha"), value.dalpha },
		{ _("option1"), value.option1 },
		{ _("option2"), value.option2 },
		{ _("option3"), value.option3 },
		{ _("option4"), value.option4 },


	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::chams_conf::chams& value)
{
	value.type = j.value(_("type"), 0);
	value.desync_type = j.value(_("desync_type"), 0);
	value.desync_type2 = j.value(_("desync_type2"), 0);
	value.color = j.value(_("color"), c_color(255, 0, 0, 255));
	value.desync_color = j.value(_("desync_color"), c_color(255, 0, 0, 255));
	value.xqz_color = j.value(_("xqz_color"), c_color(255, 0, 0, 255));
	value.xqz = j.value(_("xqz"), false);
	value.enabled = j.value(_("enabled"), false);
	value.scope_blend = j.value(_("scope_blend"), false);
	value.alpha = j.value(_("alpha"), 100.f);
	value.dalpha = j.value(_("dalpha"), 100.f);
	value.option1 = j.value(_("option1"), false);
	value.option2 = j.value(_("option2"), false);
	value.option3 = j.value(_("option3"), false);
	value.option4 = j.value(_("option4"), false);

}

inline void to_json(nlohmann::json& j, const c_config::config_conf::chams_conf& value)
{
	j = json{
		{ _("enemy"), value.enemy },
		{ _("backtrack"), value.backtrack },
		{ _("local"), value.local },
		{ _("show_desync"), value.show_desync },
		{ _("hand"), value.hand },
		{ _("weapon"), value.weapon },
		{ _("base"), value.base },
		{ _("option1"), value.option1 },
		{ _("option2"), value.option2 },
		{ _("option3"), value.option3 },
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::chams_conf& value)
{
	value.enemy = j.value(_("enemy"), c_config::config_conf::chams_conf::chams{});
	value.backtrack = j.value(_("backtrack"), c_config::config_conf::chams_conf::chams{});
	value.local = j.value(_("local"), c_config::config_conf::chams_conf::chams{});
	value.show_desync = j.value(_("show_desync"), false);
	value.option1 = j.value(_("option1"), false);
	value.option2 = j.value(_("option2"), false);
	value.option3 = j.value(_("option3"), false);
	value.base = j.value(_("base"), 0);
	value.hand = j.value(_("hand"), c_config::config_conf::chams_conf::chams{});
	value.weapon = j.value(_("weapon"), c_config::config_conf::chams_conf::chams{});

}

inline void to_json(nlohmann::json& j, const c_config::config_conf::legit_conf& value)
{
	j = json{
		{ _("assist"), value.assist },
		{ _("fov"), value.fov },
		{ _("smooth"), value.smooth },
		{ _("backtrack"), value.backtrack }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::legit_conf& value)
{
	value.assist = j.value(_("assist"), false);
	value.fov = j.value(_("fov"), 0.f);
	value.smooth = j.value(_("smooth"), 0.f);
	value.backtrack = j.value(_("backtrack"), 0.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::rage_conf::weapon_conf& value)
{
	j = json{
		{ _("hitchance_head"), value.hitchance_head },
		{ _("hitchance_body"), value.hitchance_body },
		{ _("min_dmg"), value.min_dmg },
		{ _("body_after_x_missed_spread"), value.body_after_x_missed_spread },
		{ _("body_after_x_missed_resolver"), value.body_after_x_missed_resolver },
		{ _("head_scale"), value.head_scale },
		{ _("chest_scale"), value.chest_scale },
		{ _("stomach_scale"), value.stomach_scale },
		{ _("pelvis_scale"), value.pelvis_scale },
		{ _("legs_scale"), value.legs_scale },
		{ _("feet_scale"), value.feet_scale },
		{ _("optimize_fps"), value.optimize_fps },

		{ _("body_aim_health"), value.body_aim_health },
		{ _("body_aim_key"), value.body_aim_key },
		{ _("head_aim_key"), value.head_aim_key },
		{ _("hitscan_head"), value.hitscan_head },
		{ _("hitscan_chest"), value.hitscan_chest },
		{ _("hitscan_stomach"), value.hitscan_stomach },
		{ _("hitscan_pelvis"), value.hitscan_pelvis },
		{ _("hitscan_legs"), value.hitscan_legs },
		{ _("hitscan_feet"), value.hitscan_feet },

		{ _("on_shot_hitscan_head"), value.on_shot_hitscan_head },
		{ _("on_shot_hitscan_body"), value.on_shot_hitscan_body },

		{ _("hitscan_head_moving"), value.hitscan_head_moving },
		{ _("hitscan_chest_moving"), value.hitscan_chest_moving },
		{ _("hitscan_stomach_moving"), value.hitscan_stomach_moving },
		{ _("hitscan_pelvis_moving"), value.hitscan_pelvis_moving },
		{ _("hitscan_legs_moving"), value.hitscan_legs_moving },
		{ _("hitscan_feet_moving"), value.hitscan_feet_moving },

		{ _("auto_shoot"), value.auto_shoot },
		{ _("head_aim_only_while_firing"), value.head_aim_only_while_firing },
		{ _("smart_aim"), value.smart_aim },
		{ _("override_head_aim_only"), value.override_head_aim_only },
		{ _("body_aim_in_air"), value.body_aim_in_air },
		{ _("body_aim_lethal"), value.body_aim_lethal },
		{ _("body_aim_slow_walk"), value.body_aim_slow_walk },
		{ _("body_aim_if_not_on_shot"), value.body_aim_if_not_on_shot },
		{ _("min_dmg_hp"), value.min_dmg_hp },
		{ _("min_dmg_hp_slider"), value.min_dmg_hp_slider },
		{ _("hp"), value.hp },
		{ _("hp_health_override"), value.hp_health_override },
		{ _("hp_head_hitchance"), value.hp_head_hitchance },
		{ _("hp_body_hitchance"), value.hp_body_hitchance },
		{ _("hp_mindmg"), value.hp_mindmg },
	};
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::rage_conf::fakelag_conf& value)
{
	j = json{

		{ _("enabled"), value.enabled },
		{ _("fake_lag_standing"), value.fake_lag_standing },
		{ _("fake_lag_moving"), value.fake_lag_moving },
		{ _("fake_lag_air"), value.fake_lag_air },
		{ _("fake_lag_slowwalk"), value.fake_lag_slowwalk },
		{ _("fake_lag_on_peek_delay"), value.fake_lag_on_peek_delay },
		{ _("disable_on_knife"), value.disable_on_knife },
		{ _("disable_on_tazer"), value.disable_on_tazer },
		{ _("disable_on_grenade"), value.disable_on_grenade },
		{ _("fake_lag_automatic"), value.fake_lag_automatic },
		{ _("disable_on_shooting"), value.disable_on_shooting },
		{ _("disable_on_revolver"), value.disable_on_revolver }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::rage_conf::fakelag_conf& value)
{
	value.enabled = j.value(_("enabled"), false);
	value.fake_lag_standing = j.value(_("fake_lag_standing"), 0);
	value.fake_lag_moving = j.value(_("fake_lag_moving"), 0);
	value.fake_lag_air = j.value(_("fake_lag_air"), 0);
	value.fake_lag_slowwalk = j.value(_("fake_lag_slowwalk"), 0);
	value.fake_lag_on_peek_delay = j.value(_("fake_lag_on_peek_delay"), false);
	value.disable_on_knife = j.value(_("disable_on_knife"), false);
	value.disable_on_tazer = j.value(_("disable_on_tazer"), false);
	value.disable_on_grenade = j.value(_("disable_on_grenade"), false);
	value.fake_lag_automatic = j.value(_("fake_lag_automatic"), false);
	value.disable_on_shooting = j.value(_("disable_on_shooting"), false);
	value.disable_on_revolver = j.value(_("disable_on_revolver"), false);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::rage_conf::antiaim_conf& value)
{
	j = json{
		{ _("enabled"), value.enabled },
		{ _("pitch_mode"), value.pitch_mode },
		{ _("yaw_mode"), value.yaw_mode },
		{ _("yaw_add"), value.yaw_add },
		{ _("custom_pitch"), value.custom_pitch },
		
		{ _("jitter_range"), value.jitter_range },
		{ _("jitter_speed"), value.jitter_speed },

		{ _("pitch_moving_mode"), value.pitch_moving_mode },
		{ _("yaw_moving_mode"), value.yaw_moving_mode },
		{ _("yaw_moving_add"), value.yaw_moving_add },
		{ _("custom_moving_pitch"), value.custom_moving_pitch },
		
		{ _("jitter_moving_range"), value.jitter_moving_range },
		{ _("jitter_moving_speed"), value.jitter_moving_speed },

		{ _("pitch_air_mode"), value.pitch_air_mode },
		{ _("yaw_air_mode"), value.yaw_air_mode },
		{ _("yaw_air_add"), value.yaw_air_add },
		{ _("custom_air_pitch"), value.custom_air_pitch },
		
		{ _("jitter_air_range"), value.jitter_air_range },
		{ _("jitter_air_speed"), value.jitter_air_speed },

		{ _("pitch_slow_mode"), value.pitch_slow_mode },
		{ _("yaw_slow_mode"), value.yaw_slow_mode },
		{ _("yaw_slow_add"), value.yaw_slow_add },
		{ _("custom_slow_pitch"), value.custom_slow_pitch },
		
		{ _("jitter_slow_range"), value.jitter_slow_range },
		{ _("jitter_slow_speed"), value.jitter_slow_speed },

	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::rage_conf::antiaim_conf& value)
{
	value.enabled = j.value(_("enabled"), false);

	value.pitch_mode = j.value(_("pitch_mode"), 0);
	value.yaw_mode = j.value(_("yaw_mode"), 0);
	value.yaw_add = j.value(_("yaw_add"), 0.f);
	value.custom_pitch = j.value(_("custom_pitch"), 0.f);
	
	value.jitter_range = j.value(_("jitter_range"), 0.f);
	value.jitter_speed = j.value(_("jitter_speed"), 0.f);

	value.pitch_moving_mode = j.value(_("pitch_moving_mode"), 0);
	value.yaw_moving_mode = j.value(_("yaw_moving_mode"), 0);
	value.yaw_moving_add = j.value(_("yaw_moving_add"), 0.f);
	value.custom_moving_pitch = j.value(_("custom_moving_pitch"), 0.f);
	
	value.jitter_moving_range = j.value(_("jitter_moving_range"), 0.f);
	value.jitter_moving_speed = j.value(_("jitter_moving_speed"), 0.f);

	value.pitch_air_mode = j.value(_("pitch_air_mode"), 0);
	value.yaw_air_mode = j.value(_("yaw_air_mode"), 0);
	value.yaw_air_add = j.value(_("yaw_air_add"), 0.f);
	value.custom_air_pitch = j.value(_("custom_air_pitch"), 0.f);
	;
	value.jitter_air_range = j.value(_("jitter_air_range"), 0.f);
	value.jitter_air_speed = j.value(_("jitter_air_speed"), 0.f);

	value.pitch_slow_mode = j.value(_("pitch_slow_mode"), 0);
	value.yaw_slow_mode = j.value(_("yaw_slow_mode"), 0);
	value.yaw_slow_add = j.value(_("yaw_slow_add"), 0.f);
	value.custom_slow_pitch = j.value(_("custom_slow_pitch"), 0.f);
	
	value.jitter_slow_range = j.value(_("jitter_slow_range"), 0.f);
	value.jitter_slow_speed = j.value(_("jitter_slow_speed"), 0.f);
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::rage_conf::weapon_conf& value)
{
	value.hitchance_head = j.value(_("hitchance_head"), 0.f);
	value.hitchance_body = j.value(_("hitchance_body"), 0.f);
	value.min_dmg = j.value(_("min_dmg"), 0.f);
	value.body_after_x_missed_spread = j.value(_("body_after_x_missed_spread"), 0);
	value.body_after_x_missed_resolver = j.value(_("body_after_x_missed_resolver"), 0);
	value.head_scale = j.value(_("head_scale"), 20.f);
	value.chest_scale = j.value(_("chest_scale"), 20.f);
	value.stomach_scale = j.value(_("stomach_scale"), 20.f);
	value.pelvis_scale = j.value(_("pelvis_scale"), 20.f);
	value.legs_scale = j.value(_("legs_scale"), 20.f);
	value.feet_scale = j.value(_("feet_scale"), 20.f);
	value.optimize_fps = j.value(_("optimize_fps"), 20.f);
	value.body_aim_health = j.value(_("body_aim_health"), 0);
	value.body_aim_key = j.value(_("body_aim_key"), 0);
	value.head_aim_key = j.value(_("head_aim_key"), 0);
	value.hitscan_head = j.value(_("hitscan_head"), false);
	value.hitscan_chest = j.value(_("hitscan_chest"), false);
	value.hitscan_stomach = j.value(_("hitscan_stomach"), false);
	value.hitscan_pelvis = j.value(_("hitscan_pelvis"), false);
	value.hitscan_legs = j.value(_("hitscan_legs"), false);
	value.hitscan_feet = j.value(_("hitscan_feet"), false);

	value.on_shot_hitscan_head = j.value(_("on_shot_hitscan_head"), false);
	value.on_shot_hitscan_body = j.value(_("on_shot_hitscan_body"), false);



	value.hitscan_head_moving = j.value(_("hitscan_head_moving"), false);
	value.hitscan_chest_moving = j.value(_("hitscan_chest_moving"), false);
	value.hitscan_stomach_moving = j.value(_("hitscan_stomach_moving"), false);
	value.hitscan_pelvis_moving = j.value(_("hitscan_pelvis_moving"), false);
	value.hitscan_legs_moving = j.value(_("hitscan_legs_moving"), false);
	value.hitscan_feet_moving = j.value(_("hitscan_feet_moving"), false);

	value.auto_shoot = j.value(_("auto_shoot"), false);
	value.head_aim_only_while_firing = j.value(_("head_aim_only_while_firing"), false);
	value.smart_aim = j.value(_("smart_aim"), false);
	value.override_head_aim_only = j.value(_("override_head_aim_only"), false);
	value.body_aim_in_air = j.value(_("body_aim_in_air"), false);
	value.body_aim_lethal = j.value(_("body_aim_lethal"), false);
	value.body_aim_slow_walk = j.value(_("body_aim_slow_walk"), false);
	value.body_aim_if_not_on_shot = j.value(_("body_aim_if_not_on_shot"), false);
	value.min_dmg_hp = j.value(_("min_dmg_hp"), false);
	value.min_dmg_hp_slider = j.value(_("min_dmg_hp_slider"), 1);
	value.hp = j.value(_("hp"), 0.f);
	value.hp_health_override = j.value(_("hp_health_override"), 1);
	value.hp_head_hitchance = j.value(_("hp_head_hitchance"), 1);
	value.hp_body_hitchance = j.value(_("hp_body_hitchance"), 1);
	value.hp_mindmg = j.value(_("hp_mindmg"), 1);

}

inline void to_json(nlohmann::json& j, const c_config::config_conf::rage_conf& value)
{
	j = json{

		{ _("enabled"), value.enabled },
		{ _("resolver"), value.resolver },
		{ _("slide"), value.slide },
		{ _("adaptive_history_backtrack"), value.adaptive_history_backtrack },
		{ _("experiemental_resolver"), value.experiemental_resolver },
		{ _("slow_walk"), value.slow_walk },
		{ _("fake_duck"), value.fake_duck },
		{ _("backtrack_mode"), value.backtrack_mode },
		{ _("antiaim_settings"), value.antiaim_settings },
		{ _("fakelag_settings"), value.fakelag_settings },

		{ _("auto_snipe"), value.auto_snipe },
		{ _("scout"), value.scout },
		{ _("awp"), value.awp },
		{ _("pistol"), value.pistol },
		{ _("heavy"), value.heavy },
		{ _("misc"), value.misc },

		{ _("health"), value.health },
		{ _("headonly"), value.headonly },
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::rage_conf& value)
{
	value.enabled = j.value(_("enabled"), false);
	value.resolver = j.value(_("resolver"), 0);
	value.slide = j.value(_("slide"), false);
	value.adaptive_history_backtrack = j.value(_("adaptive_history_backtrack"), false);
	value.experiemental_resolver = j.value(_("experiemental_resolver"), false);
	value.slow_walk = j.value(_("slow_walk"), 0);
	value.fake_duck = j.value(_("fake_duck"), 0);
	value.backtrack_mode = j.value(_("backtrack_mode"), 0);
	value.antiaim_settings = j.value(_("antiaim_settings"), c_config::config_conf::rage_conf::antiaim_conf{});
	value.fakelag_settings = j.value(_("fakelag_settings"), c_config::config_conf::rage_conf::fakelag_conf{});

	value.auto_snipe = j.value(_("auto_snipe"), c_config::config_conf::rage_conf::weapon_conf{});
	value.scout = j.value(_("scout"), c_config::config_conf::rage_conf::weapon_conf{});
	value.awp = j.value(_("awp"), c_config::config_conf::rage_conf::weapon_conf{});
	value.pistol = j.value(_("pistol"), c_config::config_conf::rage_conf::weapon_conf{});
	value.heavy = j.value(_("heavy"), c_config::config_conf::rage_conf::weapon_conf{});
	value.misc = j.value(_("misc"), c_config::config_conf::rage_conf::weapon_conf{});
	value.health = j.value(_("health"), 0);
	value.headonly = j.value(_("headonly"), 0);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::misc_conf& value)
{
	j = json{
			{ _("inverter"), value.inverter },
			{ _("flip"), value.flip },


		{ _("gravity"), value.gravity },
		{ _("knife"), value.knife },


		{ _("on_screen"), value.on_screenplayers },
		{ _("on_screenx"), value.on_screen_x },
		{ _("on_screeny"), value.on_screen_y },
		{ _("on_ind_x"), value.on_ind_x },
		{ _("on_ind_y"), value.on_ind_y },

		{ _("slow_walk_speed"), value.slow_walk_speed },
		{ _("radio_channel"), value.radio_channel },
		{ _("radio_volume"), value.radio_volume },
		{ _("radio_mute"), value.radio_mute },

		{ _("spike_ping"), value.spike_ping },
		{ _("ping_switch"), value.ping_switch },

		{ _("thirdperson_dist"), value.thirdperson_dist },
		{ _("thirdperson_switch"), value.thirdperson_switch },

		{ _("buy_bot"), value.buy_bot },
		{ _("buy_bot_primary"), value.buy_bot_primary },
		{ _("buy_bot_secondary"), value.buy_bot_secondary },
		{ _("buy_bot_armor"), value.buy_bot_armor },
		{ _("buy_bot_grenades"), value.buy_bot_grenades },
		{ _("buy_bot_zeus"), value.buy_bot_zeus },
		{ _("buy_bot_defuser"), value.buy_bot_defuser },

		{ _("engine_radar"), value.engine_radar },
		{ _("no_recoil"), value.no_recoil },
		{ _("no_smoke"), value.no_recoil },
		{ _("no_flash"), value.no_flash },
		{ _("no_scope"), value.no_scope },
		{ _("no_draw"), value.no_draw },
		{ _("no_post_processing"), value.no_post_processing },
		{ _("no_fog"), value.no_fog},
		{ _("remove_zoom"), value.remove_zoom},
		{ _("remove_punch"), value.remove_punch},
		{ _("indicators"), value.indicators },
		{ _("arrows"), value.arrows },
		{ _("nightmode"), value.nightmode },
		{ _("nightmode_selection"), value.nightmode_selection },
		{ _("nightmode_color"), value.nightmode_color },
		{ _("nightmode_darkness"), value.nightmode_darkness },

		{ _("full_bright"), value.full_bright },
		{ _("sv_impacts"), value.sv_impacts },
		{ _("preserve_feed"), value.preserve_feed },
		{ _("fov"), value.fov },
		{ _("fov_view"), value.fov_view },
		{ _("transparency"), value.transparency },
		{ _("clantag"), value.clantag },

		{ _("hotkey_x"), value.hotkey_x },
		{ _("hotkey_y"), value.hotkey_y },
		{ _("hotkeyss"), value.hotkeyss },
		{ _("watermark"), value.watermark },
		{ _("theme"), value.theme },

		{ _("indicator_antiaim"), value.indicator_antiaim },
		{ _("indicator_antiaiml"), value.indicator_antiaim_l },
		{ _("indicator_antiaimc"), value.indicator_antiaimc },
		
		{ _("streamermode"), value.streamermode },
		{ _("hit_effect"), value.hit_effect },
		{ _("kill_effect"), value.kill_effect },
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::misc_conf& value)
{
	value.inverter = j.value(_("inverter"), 0);
	value.flip = j.value(_("flip"), 0);

	value.gravity = j.value(_("gravity"), false);
	value.knife = j.value(_("knife"), false);

	value.on_screenplayers = j.value(_("on_screen"), false);
	value.on_screen_x = j.value(_("on_screenx"), 500);
	value.on_screen_y = j.value(_("on_screenx"), 500);
	value.on_ind_x = j.value(_("on_ind_x"), 5);
	value.on_ind_y = j.value(_("on_ind_y"), 500);

	value.slow_walk_speed = j.value(_("slow_walk_speed"), 0);
	value.radio_channel = j.value(_("radio_channel"), 0);
	value.radio_volume = j.value(_("radio_volume"), 100.f);
	value.radio_mute = j.value(_("radio_mute"), 0);

	value.spike_ping = j.value(_("spike_ping"), 0.f);
	value.ping_switch = j.value(_("ping_switch"), false);

	value.thirdperson_dist = j.value(_("thirdperson_dist"), 0.f);
	value.thirdperson_switch = j.value(_("thirdperson_switch"), 0);

	value.buy_bot = j.value(_("buy_bot"), false);
	value.buy_bot_primary = j.value(_("buy_bot_primary"), 0);
	value.buy_bot_secondary = j.value(_("buy_bot_secondary"), 0);
	value.buy_bot_armor = j.value(_("buy_bot_armor"), 0);
	value.buy_bot_grenades = j.value(_("buy_bot_grenades"), false);
	value.buy_bot_zeus = j.value(_("buy_bot_zeus"), false);
	value.buy_bot_defuser = j.value(_("buy_bot_defuser"), false);

	value.engine_radar = j.value(_("engine_radar"), false);
	value.no_recoil = j.value(_("no_recoil"), false);
	value.no_smoke = j.value(_("no_smoke"), false);
	value.no_flash = j.value(_("no_flash"), false);
	value.no_scope = j.value(_("no_scope"), false);
	value.no_draw = j.value(_("no_draw"), false);
	value.no_post_processing = j.value(_("no_post_processing"), false);
	value.no_fog = j.value(_("no_fog"), false);
	value.remove_zoom = j.value(_("remove_zoom"), false);
	value.remove_punch = j.value(_("remove_punch"), false);
	value.indicators = j.value(_("indicators"), false);
	value.arrows = j.value(_("arrows"), false);
	value.nightmode = j.value(_("nightmode"), false);
	value.nightmode_selection = j.value(_("nightmode_selection"), 0);
	value.nightmode_color = j.value(_("nightmode_color"), c_color(255, 0, 0, 255));
	value.nightmode_darkness = j.value(_("nightmode_darkness"), 0.f);
	value.full_bright = j.value(_("full_bright"), false);
	value.sv_impacts = j.value(_("sv_impacts"), false);
	value.preserve_feed = j.value(_("preserve_feed"), false);
	value.fov = j.value(_("fov"), 50.f);
	value.fov_view = j.value(_("fov_view"), 50.f);
	value.transparency = j.value(_("transparency"), 100.f);
	value.clantag = j.value(_("clantag"), false);

	value.hotkey_x = j.value(_("hotkey_x"), 100);
	value.hotkey_y = j.value(_("hotkey_y"), 100);
	value.hotkeyss = j.value(_("hotkeyss"), false);
	value.watermark = j.value(_("watermark"), true);
	value.theme = j.value(_("theme"), c_color(255, 200, 50, 255));

	value.indicator_antiaim = j.value(_("indicator_antiaim"), false);
	value.indicator_antiaim_l = j.value(_("indicator_antiaiml"), false);
	value.indicator_antiaimc = j.value(_("indicator_antiaimc"), c_color(255, 200, 50, 255));
	value.streamermode = j.value(_("streamermode"), false);

	value.hit_effect = j.value(_("hit_effect"), false);
	value.kill_effect = j.value(_("kill_effect"), false);

}

inline void to_json(nlohmann::json& j, const c_config::config_conf& value)
{
	j = json{
		{ _("esp"), value.esp },
		{ _("chams"), value.chams },
		{ _("legit"), value.legit },
		{ _("rage"), value.rage },
		{ _("misc"), value.misc },
		{ _("version_number"), value.version_number }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf& value)
{
	value.esp = j.value(_("esp"), c_config::config_conf::esp_conf{});
	value.chams = j.value(_("chams"), c_config::config_conf::chams_conf{});
	value.legit = j.value(_("legit"), c_config::config_conf::legit_conf{});
	value.rage = j.value(_("rage"), c_config::config_conf::rage_conf{});
	value.misc = j.value(_("misc"), c_config::config_conf::misc_conf{});
	value.version_number = j.value(_("version_number"), 1.2f);

}

inline void c_config::load_from_file(const std::string& config_path)
{

	std::ifstream file(config_path);
	std::stringstream rage;
	rage << file.rdbuf();

	if (rage.str().empty())
		return;

	auto cfg = base64::decode(rage.str());

	try {
		from_json(json::parse(cfg), instance()->get());
	}
	catch (json::exception exc) {
		cvar()->console_color_printf(false, c_color(255, 100, 100), "[load][%d]%s\n", exc.id, exc.what());
	}
}

inline void c_config::save_to_file(const std::string& config_path)
{
	json exp;
	try {
		to_json(exp, instance()->get());
	}
	catch (json::exception exc) {
		cvar()->console_color_printf(false, c_color(255, 100, 100), "[load][%d]%s\n", exc.id, exc.what());
	}
	const auto str = base64::encode(exp.dump());

	std::ofstream file(config_path);
	file << str;
}

#define config c_config::instance()->get()
