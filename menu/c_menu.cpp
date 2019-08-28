#include "c_menu.h"
#include "c_visuals.h"
#include "c_aiming.h"
#include "../fonts/profont.h"
#include "c_misc.h"

#include "../hooks/idirect3ddevice9.h"
#include "../sdk/c_input_system.h"
#include "../hacks/c_antiaim.h"
#include "../sdk/c_game_rules.h"
#include "../sdk/c_cs_player.h"
#include "../images/logo.h"
#include "c_conf.h"
#include <memory>
#include "../bobhack_main.h"
#include "../security/string_obfuscation.h"
#include "../sdk/c_debug_overlay.h"
#include <d3d9.h>
#include "../imgui/imgui.h"
#include "../ImGUI/DX9/imgui_impl_dx9.h"
#include "../imgui/imgui_internal.h"
#include "../ImConsole.h"
#include "../LuaAPI.h"
#include "../TextEditor.h"
#include "../hacks/c_aimhelper.h"


#include "../loki_framework.h"
#include "../RadioManager.h"

#define RGBA_TO_FLOAT(r,g,b,a) (float)r/255.0f, (float)g/255.0f, (float)b/255.0f, (float)a/255.0f

ImFont* cheat_font;
ImFont* tab_font;
ImFont* tab_font_text;
ImFont* smallcock;

const char* const KeyNames[] = {
	"Unknown",
	"M1",
	"M2",
	"CAN",
	"M3",
	"M4",
	"M4",
	"Unknown",
	"BACK",
	"TAB",
	"Unknown",
	"Unknown",
	"CLEAR",
	"RETURN",
	"Unknown",
	"Unknown",
	"SHI",
	"CTRL",
	"MENU",
	"PAUSE",
	"CAPS",
	"KANA",
	"Unknown",
	"JUNJA",
	"FINAL",
	"KANJI",
	"Unknown",
	"ESC",
	"CONV",
	"NCONV",
	"ACC",
	"MC",
	"SPACE",
	"PRIOR",
	"NEXT",
	"END",
	"HOME",
	"LEFT",
	"UP",
	"RIGHT",
	"DOWN",
	"VK_SELECT",
	"PRINT",
	"EXECUTE",
	"SNAPSHOT",
	"INS",
	"DEL",
	"HELP",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"LWIN",
	"RWIN",
	"APPS",
	"Unknown",
	"SLEEP",
	"NP0",
	"NP1",
	"NP2",
	"NP3",
	"NP4",
	"NP5",
	"NP6",
	"NP7",
	"NP8",
	"NP9",
	"MULTI",
	"ADD",
	"SEP",
	"SUN",
	"DECI",
	"DIVI",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"F16",
	"F17",
	"F18",
	"F19",
	"F20",
	"F21",
	"F22",
	"F23",
	"F24",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"NML",
	"SCROLL",
	"UK",
	"UK",
	"UK",
	"UK",
	"UK",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"LSHI",
	"RSHI",
	"LCTRL",
	"RCTRL",
	"LMENU",
	"RMENU"
};




c_menu::c_menu() : c_flow_layout(renderer->get_center() * 2.f), open(false)
{
	hotkeys->register_callback([&](auto code) -> void
	{
		if (code != VK_INSERT || (alpha > 0 && alpha < 255))
			return;

		open = !open;
		// reset hold
		if (current_held.has_value())
		{
			current_held.value()->held = false;
			set_current_held(std::nullopt);
		}

		input_system()->enable_input(!open);
		input_system()->reset_input_state();
	});
}

bool c_menu::is_open() const
{
	return open;
}

void c_menu::GUI_Init(HWND window, IDirect3DDevice9 * pDevice)
{
IMGUI_CHECKVERSION();

ImGui::CreateContext();

ImGui_ImplDX9_Init(window, pDevice);

ImGuiStyle * style = &ImGui::GetStyle();
ImGuiIO & io = ImGui::GetIO();

io.IniFilename = "imgui.ini";

ImFontConfig font_config;

font_config.OversampleH = font_config.OversampleV = 1;
font_config.PixelSnapH = 1;

static const ImWchar ranges[] =
{
	0x0020, 0x00FF,
	0x0400, 0x044F,
	0,
};

smallcock = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 9, &font_config, ranges);
cheat_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16, &font_config, ranges);
tab_font = io.Fonts->AddFontFromFileTTF("C:\\bobhack\\resources\\icons.ttf", 20, &font_config, ranges);
tab_font_text = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16, &font_config, ranges);

ImGui::StyleColorsDark();

if (!image)
renderer->create_texture(logo, sizeof(logo), &image);

if (!sprite)
renderer->create_sprite(&sprite);



style->Colors[ImGuiCol_Text] = ImVec4(RGBA_TO_FLOAT(180, 180, 180, 255));
style->Colors[ImGuiCol_WindowBg] = ImVec4(RGBA_TO_FLOAT(24, 24, 24, 255));
style->Colors[ImGuiCol_Border] = ImVec4(RGBA_TO_FLOAT(0, 0, 0, 255));
style->Colors[ImGuiCol_FrameBg] = ImVec4(.18f, .18f, .18f, 1.f);
style->Colors[ImGuiCol_Theme] = ImVec4(RGBA_TO_FLOAT(90, 50, 200, 255));


style->Alpha = 1.f;
style->WindowTitleAlign = ImVec2(0.5, 0.5);
style->WindowRounding = 0.0f;
style->ItemSpacing = ImVec2(4, 4);
SetCursor(nullptr);


d3dinit = true;

CONSOLE->Init();
}

float c_menu::lerp(float t, float a, float b) {
	return (1 - t)*a + t * b;
}



std::vector<std::string> get_all_files_names_within_folder(std::string folder)
{
	std::vector<std::string> names = { "rage", "rage2", "legithacc", "epictrio", "serverside", "nospread" };
	std::string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}


void c_menu::watermark() {
	if (!config.misc.watermark) return;
	//auto p = c_loki::watermark(open);
	//config.misc.on_ind_x = p.x;
	//config.misc.on_ind_y = p.y;
}


void c_menu::mainWindow()
{
	
	if (!open) return;
	framework::run();
	switch (framework::tabs({ "ragebot", "\"legitbot\"", "antiaim", "esp", "models",  "miscellaneous" })) {
	case 0: {
		static int wep = 0;
		auto cfg = &config.rage.auto_snipe;
		switch (wep) {
		case 0: cfg = &config.rage.auto_snipe; break;
		case 1: cfg = &config.rage.scout; break;
		case 2: cfg = &config.rage.awp; break;
		case 3: cfg = &config.rage.pistol; break;
		case 4: cfg = &config.rage.misc; break;
		}

		if (framework::groupbox("ragebot", 116, 25)){
			framework::checkbox("active", &config.rage.enabled);
			framework::combo("editing weapon", &wep, { "auto", "scout", "awp", "pistol", "default" });
			framework::checkbox("extended backtrack", &config.misc.ping_switch);
			framework::combo("resolver mode", &config.rage.resolver, { "off", "stable", "bruteforce beta" });
			framework::combo("backtrack mode", &config.rage.backtrack_mode, { "best", "last" });
			
		}
		framework::end_groupbox();

		if (framework::groupbox("accuracy", 115)) {
			framework::slider("head hitchance", &cfg->hitchance_head, 0.f, 100.f, "%");
			framework::slider("body hitchance", &cfg->hitchance_body, 0.f, 100.f, "%");
			framework::slider("minimum damage", &cfg->min_dmg, 0.f, 100.f, "hp");
			framework::slider("fps optimization", &cfg->optimize_fps, 0.f, 300.f, "");
		}
		framework::end_groupbox();
		
		if (framework::groupbox("pointscale", -1)) {

			framework::slider("head", &cfg->head_scale, 0.f, 100.f, "%");
			framework::slider("chest", &cfg->chest_scale, 0.f, 100.f, "%");		
			framework::slider("stomach", &cfg->stomach_scale, 0.f, 100.f, "%");
			framework::slider("pelvis", &cfg->pelvis_scale, 0.f, 100.f, "%");
			framework::slider("legs", &cfg->legs_scale, 0.f, 100.f, "%");		
			framework::slider("feet", &cfg->feet_scale, 0.f, 100.f, "%");
			
		}
		framework::end_groupbox();

		framework::next_column();
		if (framework::groupbox("hitboxes", 137)) {
			static int mode = 0;
			framework::combo("mode", &mode, { "standing", "moving" });
			if (mode == 0) {
				framework::checkbox("head", &cfg->hitscan_head);
				framework::checkbox("chest", &cfg->hitscan_chest);
				framework::checkbox("stomach", &cfg->hitscan_stomach);
				framework::checkbox("pelvis", &cfg->hitscan_pelvis);
				framework::checkbox("legs", &cfg->hitscan_legs);
				framework::checkbox("feet", &cfg->hitscan_feet);
			}
			else {
				framework::checkbox("head", &cfg->hitscan_head_moving);
				framework::checkbox("chest", &cfg->hitscan_chest_moving);
				framework::checkbox("stomach", &cfg->hitscan_stomach_moving);
				framework::checkbox("pelvis", &cfg->hitscan_pelvis_moving);
				framework::checkbox("legs", &cfg->hitscan_legs_moving);
				framework::checkbox("feet", &cfg->hitscan_feet_moving);
			}
			
		}
		framework::end_groupbox();

		if (framework::groupbox("health override", -1)) {
			framework::checkbox("active", &cfg->min_dmg_hp);
			framework::slider("damage", &cfg->min_dmg_hp_slider, 1, 5, "");
			framework::slider("health", &cfg->hp, 1.f, 100.f, "");
			framework::slider("override health", &cfg->hp_health_override, 1.f, 100.f, "");
			framework::slider("override head hitchance", &cfg->hp_head_hitchance, 1.f, 100.f, "%");
			framework::slider("override body hitchance", &cfg->hp_body_hitchance, 1.f, 100.f, "%");
			framework::slider("override min damage", &cfg->hp_mindmg, 1.f, 100.f, "");
		}
		framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("filters", 137)) {
			framework::hotkey("force head", &cfg->head_aim_key);
			framework::hotkey("force body", &cfg->body_aim_key);
			// head hotkey
			// body hotkey
			framework::slider("resolver miss fbaim", &cfg->body_after_x_missed_resolver, 0, 5, "");
			framework::slider("spread miss fbaim", &cfg->body_after_x_missed_spread, 0, 5, "");

		}
		framework::end_groupbox();

		if (framework::groupbox("bodyaim", 137)) {
			framework::checkbox("lethal", &cfg->body_aim_lethal);
			framework::checkbox("in-air", &cfg->body_aim_in_air);
			framework::checkbox("slow walking", &cfg->body_aim_slow_walk);
			framework::checkbox("not shooting", &cfg->body_aim_if_not_on_shot);

		}
		framework::end_groupbox();
		if (framework::groupbox("on-shot", -1)) {
			framework::checkbox("active", &cfg->head_aim_only_while_firing);
			framework::checkbox("prioritize on-shot scan", &cfg->override_head_aim_only);
			framework::checkbox("hitscan head on-shot", &cfg->on_shot_hitscan_head);
			framework::checkbox("hitscan body on-shot", &cfg->on_shot_hitscan_body);
			

		}
		framework::end_groupbox();
	} break;
	case 1: {
		if (framework::groupbox("legitbot", -1)) {
			framework::text("it's so bad!");
			framework::checkbox("active", &config.legit.assist);
			
		} framework::end_groupbox();
	} break;
	case 2: {
		if (framework::groupbox("antiaim", 100)) {
			framework::checkbox("active", &config.rage.antiaim_settings.enabled);
			framework::hotkey("slow walk", &config.rage.slow_walk);
			framework::hotkey("bobduck", &config.rage.fake_duck);
			framework::hotkey("inverter(WIP)", &config.misc.inverter);
			framework::hotkey("flip manual(WIP)", &config.misc.flip);
			// config slow_walk
			// config fake_duck
			// config misc.inverter
			// config misc.flip

		}
		framework::end_groupbox();
	
		if (framework::groupbox("fakelag", -1)) {
			framework::checkbox("enabled", &config.rage.fakelag_settings.enabled);
			framework::checkbox("adaptive", &config.rage.fakelag_settings.fake_lag_automatic);
			framework::slider("standing", &config.rage.fakelag_settings.fake_lag_standing, 1, 16, "t");
			framework::slider("moving", &config.rage.fakelag_settings.fake_lag_moving, 1, 16, "t");
			framework::slider("in air", &config.rage.fakelag_settings.fake_lag_air, 1, 16, "t");
			framework::slider("slow walking", &config.rage.fakelag_settings.fake_lag_slowwalk, 1, 16, "t");

			framework::checkbox("peek delay", &config.rage.fakelag_settings.fake_lag_on_peek_delay);
			framework::checkbox("disable on shot", &config.rage.fakelag_settings.disable_on_shooting);
			framework::checkbox("disable on revolver", &config.rage.fakelag_settings.disable_on_revolver);
			framework::checkbox("disable on taser ", &config.rage.fakelag_settings.disable_on_tazer);
			framework::checkbox("disable on grenade", &config.rage.fakelag_settings.disable_on_grenade);
			framework::checkbox("disable on knife", &config.rage.fakelag_settings.disable_on_knife);

		}
		framework::end_groupbox();

		framework::next_column();
		if (framework::groupbox("standing", 210)) {
			if (framework::combo("pitch", &config.rage.antiaim_settings.pitch_mode, { "off", "down", "zero", "up", "custom" }) == 4)
				framework::slider("custom pitch", &config.rage.antiaim_settings.custom_pitch, 0.f, 360.f, "");
			framework::combo("yaw", &config.rage.antiaim_settings.yaw_mode, { "off","static", "jitter", "freestanding", "manual" });
			framework::slider("yaw offset", &config.rage.antiaim_settings.yaw_add, 0.f, 360.f, "");
			if (config.rage.antiaim_settings.yaw_mode == 2)
				framework::slider("jitter range", &config.rage.antiaim_settings.jitter_range, 0.f, 180.f, "");
		}
		framework::end_groupbox();
		if (framework::groupbox("moving", -1)) {
			if (framework::combo("pitch", &config.rage.antiaim_settings.pitch_moving_mode, { "off", "down", "zero", "up", "custom" }) == 4)
				framework::slider("custom pitch", &config.rage.antiaim_settings.custom_moving_pitch, 0.f, 360.f, "");
			framework::combo("yaw", &config.rage.antiaim_settings.yaw_moving_mode, { "off","static", "jitter", "freestanding", "manual" });
			framework::slider("yaw offset", &config.rage.antiaim_settings.yaw_moving_add, 0.f, 360.f, "");
			if (config.rage.antiaim_settings.yaw_moving_mode == 2)
				framework::slider("jitter range", &config.rage.antiaim_settings.jitter_moving_range, 0.f, 180.f, "");
		}
		framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("slow walking", 210)) {
			if (framework::combo("pitch", &config.rage.antiaim_settings.pitch_slow_mode, { "off", "down", "zero", "up", "custom" }) == 4)
				framework::slider("custom pitch", &config.rage.antiaim_settings.custom_slow_pitch, 0.f, 360.f, "");
			framework::combo("yaw", &config.rage.antiaim_settings.yaw_slow_mode, { "off","static", "jitter", "freestanding", "manual" });
			framework::slider("yaw offset", &config.rage.antiaim_settings.yaw_slow_add, 0.f, 360.f, "");
			if (config.rage.antiaim_settings.yaw_slow_mode == 2)
				framework::slider("jitter range", &config.rage.antiaim_settings.jitter_slow_range, 0.f, 180.f, "");
		}
		framework::end_groupbox();
		if (framework::groupbox("in-air", -1)) {
			if (framework::combo("pitch", &config.rage.antiaim_settings.pitch_air_mode, { "off", "down", "zero", "up", "custom" }) == 4)
				framework::slider("custom pitch", &config.rage.antiaim_settings.custom_air_pitch, 0.f, 360.f, "");
			framework::combo("yaw", &config.rage.antiaim_settings.yaw_air_mode, { "off","static", "jitter", "freestanding", "manual" });
			framework::slider("yaw offset", &config.rage.antiaim_settings.yaw_air_add, 0.f, 360.f, "");
			if (config.rage.antiaim_settings.yaw_air_mode == 2)
				framework::slider("jitter range", &config.rage.antiaim_settings.jitter_moving_range, 0.f, 180.f, "");
		}
		framework::end_groupbox();
	} break;
	case 3: {
		if (framework::groupbox("players", 122)) {
			framework::checkbox("name tag", &config.esp.enemy.name);
			framework::colors(&config.esp.name_color);

			framework::checkbox("bounding box", &config.esp.enemy.box);
			framework::colors(&config.esp.box_color);

			framework::checkbox("health bar", &config.esp.enemy.health);
			framework::colors(&config.esp.health_color);

			framework::checkbox("armor bar", &config.esp.armor);
			framework::colors(&config.esp.armor_color);

			framework::checkbox("weapon", &config.esp.enemy.weapon);
			framework::colors( &config.esp.weapon_color);

			framework::checkbox("ammo", &config.esp.enemy.ammo);
			framework::colors(&config.esp.ammo_color);

			framework::checkbox("skeleton", &config.esp.enemy.skeleton);
			framework::colors(&config.esp.skeleton_color);
		} framework::end_groupbox();
		if (framework::groupbox("player flags", 93)) {
			framework::checkbox("scoped", &config.esp.enemy_flags.zoom);
			framework::checkbox("reload", &config.esp.enemy_flags.reload);
			framework::checkbox("defuse", &config.esp.enemy_flags.defuse);
			framework::checkbox("armor", &config.esp.enemy_flags.kevlar);
			framework::checkbox("shots", &config.esp.enemy_flags.resolver);
		} framework::end_groupbox();
		if (framework::groupbox("other", -1)) {
			framework::checkbox("health based bar", &config.esp.health_based);
			framework::checkbox("weapon icons", &config.esp.wep_icons);
			framework::checkbox("backtrack skeleton", &config.esp.enemy.history_skeleton);
			framework::checkbox("color mode", &config.misc.nightmode);
			//framework::slider("darkness", &config.misc.nightmode_darkness, 1, 100, "");
			framework::colors(&config.misc.nightmode_color);
			framework::checkbox("full bright", &config.misc.full_bright);		
		} framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("other plays", 80)) {
			framework::checkbox("enemy bullet tracers", &config.esp.enemy.impacts);
			framework::colors(&config.esp.impacts_color);
			framework::checkbox("visualize lag", &config.esp.show_lag_compensation);
			framework::colors(&config.esp.show_lag_compensation_color);
			framework::checkbox("visualize multipoint", &config.esp.multipoint);
			framework::checkbox("visualize on-shot", &config.esp.show_on_shot_hitboxes);
		} framework::end_groupbox();
		if (framework::groupbox("removals", -1)) {
			framework::checkbox("scope overlay", &config.misc.no_scope);
			framework::checkbox("smoke", &config.misc.no_smoke);
			framework::checkbox("fog", &config.misc.no_fog);
			framework::checkbox("flashbang effects", &config.misc.no_flash);
			framework::checkbox("zoom", &config.misc.remove_zoom);
			framework::checkbox("recoil", &config.misc.no_recoil);
			framework::checkbox("punch", &config.misc.remove_punch);
			framework::checkbox("post processing", &config.misc.no_post_processing);
		} framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("other esp", -1)) {
			framework::checkbox("local bullet tracers", &config.esp.local_impact);
			framework::colors(&config.esp.local_impacts_color);
			framework::slider("thickness", &config.esp.local_impact_width, 0, 20, "");
			framework::checkbox("grenades", &config.esp.nade_esp);
			framework::colors(&config.esp.nade_color);
			framework::checkbox("grenade path", &config.esp.grenade_pred);
			framework::colors(&config.esp.grenade_predc);
		} framework::end_groupbox();
	} break;
	case 4: {
		if (framework::groupbox("custom material", 96)) {
			framework::combo("base material", &config.chams.base, { "textured", "flat", "metallic", "glow", "ray" });
			framework::checkbox("keep original model", &config.chams.option3);
			framework::checkbox("wireframe", &config.chams.option1);
			framework::checkbox("pulse", &config.chams.option2);
		} framework::end_groupbox();

		if (framework::groupbox("enemy", 134)) {
			framework::checkbox("active", &config.chams.enemy.enabled);
			framework::colors(&config.chams.enemy.color);

			framework::checkbox("two pass", &config.chams.enemy.xqz);
			framework::colors(&config.chams.enemy.xqz_color);

			framework::combo("material", &config.chams.enemy.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.enemy.type == 5 && config.chams.option2)
				framework::checkbox("preserve original", &config.chams.enemy.option1);
		} framework::end_groupbox();
		if (framework::groupbox("backtrack", 134)) {
			framework::checkbox("active", &config.chams.backtrack.enabled);

			framework::colors(&config.chams.backtrack.color);

			framework::checkbox("two pass", &config.chams.backtrack.xqz);
			framework::colors(&config.chams.backtrack.xqz_color);
			framework::combo("material", &config.chams.backtrack.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.backtrack.type == 5 && config.chams.option2)
				framework::checkbox("preserve original", &config.chams.backtrack.option1);

		} framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("local", 134)) {
			framework::checkbox("active", &config.chams.local.enabled);
			framework::colors( &config.chams.local.color);
			framework::combo("real", &config.chams.local.desync_type, { "off", "textured", "flat", "metallic", "glow", "ray", "custom" });
			framework::combo("desync", &config.chams.local.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			framework::colors(&config.chams.local.desync_color);
			if (config.chams.local.type == 5 && config.chams.option2)
				framework::checkbox("preserve original", &config.chams.local.option1);
			if (config.chams.local.desync_type == 6 && config.chams.option2)
				framework::checkbox("preserve original desync", &config.chams.local.option2);
		} framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("hands", 134)) {
			framework::checkbox("active", &config.chams.hand.enabled);
			framework::colors(&config.chams.hand.color);
			framework::combo("material", &config.chams.hand.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.hand.type == 5 && config.chams.option2)
				framework::checkbox("preserve original", &config.chams.hand.option1);
			framework::checkbox("no hands", &config.chams.hand.option2);
			framework::checkbox("no sleeves", &config.chams.hand.option3);
		} framework::end_groupbox();
		if (framework::groupbox("weapon", 134)) {
			framework::checkbox("active", &config.chams.weapon.enabled);
			framework::colors(&config.chams.weapon.color);
			framework::combo("material", &config.chams.weapon.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.weapon.type == 5 && config.chams.option2)
				framework::checkbox("preserve original", &config.chams.weapon.option1);
		} framework::end_groupbox();
	} break;
	case 5: {
		if (framework::groupbox("general", 164)) {
			framework::checkbox("watermark", &config.misc.watermark);
			framework::checkbox("round dmg shit", &config.misc.on_screenplayers);
			framework::checkbox("ragdoll meme", &config.misc.gravity);
			framework::checkbox("save killfeed", &config.misc.preserve_feed);
			framework::checkbox("bobhack spammer", &config.misc.clantag);
			static std::vector<std::string> channels;
			static bool _____A_A_A_A_A = false;
			if (!_____A_A_A_A_A) {

				channels.push_back("off");
				for (auto x : g_RadioManager.stations) {
					channels.push_back(x.Name);
				}

				_____A_A_A_A_A = true;
			}


			framework::combo("copyrighted music", &config.misc.radio_channel, channels);
			framework::slider("radio volume", &config.misc.radio_volume, 0.f, 100.f, "");
		} framework::end_groupbox();

		if (framework::groupbox("player damage", 134)) {
			framework::checkbox("hitmarker", &config.esp.hitmarker);
			framework::colors(&config.esp.hitmarker_color);
			framework::checkbox("damage indicator", &config.esp.hitmarker_damage);
			framework::checkbox("hitsound", &config.esp.hitsound);
			framework::slider("hitsound volume", &config.esp.hitsound_volume, 0.f, 100.f, "");
			framework::checkbox("hit effect", &config.misc.hit_effect);
			framework::checkbox("kill effect", &config.misc.kill_effect);
		} framework::end_groupbox();
		if (framework::groupbox("movement", -1)) {
			framework::checkbox("auto jump", &config.misc.knife);
		} framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("view", 106)) {
			framework::slider("field of view", &config.misc.fov, 0.f, 150.f, "");
			framework::slider("v-field of view", &config.misc.fov_view, 0.f, 150.f, "");
			framework::hotkey("thirdperson", &config.misc.thirdperson_switch);
			framework::slider("distance", &config.misc.thirdperson_dist, 0.f, 200.f, "");
		} framework::end_groupbox();
		if (framework::groupbox("indicators", 100)) {
			framework::checkbox("active", &config.misc.indicators);
			framework::checkbox("out of view", &config.esp.enemy.radar);
			framework::colors(&config.esp.radar_color);

			framework::checkbox("antiaim", &config.misc.indicator_antiaim);
			framework::colors(&config.misc.indicator_antiaimc);
			framework::checkbox("antiaim lines", &config.misc.indicator_antiaim_l);
		} framework::end_groupbox();
		if (framework::groupbox("buybot", -1)) {
			framework::checkbox("active", &config.misc.buy_bot);
			framework::combo("primary", &config.misc.buy_bot_primary, { "none", "auto", "scout", "awp" });
			framework::combo("secondary", &config.misc.buy_bot_secondary, { "none", "elites", "p250", "auto pistol", "heavy pistol" });
			framework::combo("armor", &config.misc.buy_bot_armor, { "none", "kevlar", "helmet + kevlar" });
			framework::checkbox("grenades", &config.misc.buy_bot_grenades);
			framework::checkbox("zeus", &config.misc.buy_bot_zeus);
			framework::checkbox("defuse kit", &config.misc.buy_bot_defuser);
		} framework::end_groupbox();
		framework::next_column();
		if (framework::groupbox("config", -1)) {
			framework::text("menu theme"); framework::colors(&config.misc.theme);
			static int cfg = 0;
			static int combo_action = 0;
			std::vector<std::string> configs = get_all_files_names_within_folder("C:\\bobhack");
			framework::combo("config", &cfg, configs);
			framework::combo("action", &combo_action, {"this is temporary", "load", "save"});


			if (combo_action != 0) {
				if (combo_action == 2) {
					std::string l_path = std::string("C:\\") + "\\bobhack\\" + configs[cfg];
					c_config::save_to_file(l_path);
					combo_action = 0;
				}
				else if (combo_action == 1) {
					std::string l_path = std::string("C:\\") + "\\bobhack\\" + configs[cfg];
					c_config::load_from_file(l_path);
					combo_action = 0;
				}
			}

		} framework::end_groupbox();
	} break;
	}



	framework::run_combo();
	framework::run_multicombo();
	framework::run_color();
	framework::run_color_dropdown();
	framework::run_color_drag();
	
	
}


