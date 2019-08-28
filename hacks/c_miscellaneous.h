#pragma once
#include "../hooks/c_view_render_.h"
#include "../sdk/c_user_cmd.h"

class c_miscellaneous
{
public:
	static void disable_post_processing();
	static void unlock_client_command_buffer();
	static void remove_visual_recoil(c_view_setup& view);
	static void change_fov(c_view_setup& view);
	static void set_viewmodel_parameters();
	static void set_camera_to_thirdperson();
	static void aspectratio();
	static void pushscale();
	static void gravity();
	static void remove_smoke();
	static void remove_flash();
	static void set_buttons_for_direction(c_user_cmd* cmd);
	static void remove_duck_stamina(c_user_cmd* cmd);
	static void engine_radar();
	static void clantag();
	static void Instastop(c_user_cmd* cmd);

};
extern c_miscellaneous g_c_miscellaneous;
