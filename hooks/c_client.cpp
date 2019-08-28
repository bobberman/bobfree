#include "../menu/c_menu.h"
#include "c_client.h"
#include "../sdk/c_material_system.h"
#include "../utils/c_hook.h"
#include "../hacks/c_animation_system.h"
#include "../sdk/c_client_entity_list.h"
#include "../sdk/c_game_rules.h"
#include "../sdk/c_weapon_system.h"
#include "../hacks/c_esp.h"
#include "../sdk/c_engine_trace.h"
#include "../sdk/c_debug_overlay.h"
#include "c_net_channel_.h"
#include "../hacks/c_movement.h"
#include "../hacks/c_resolver.h"
#include "../hacks/debug.h"
#include "../hacks/c_miscellaneous.h"
#include "../hacks/c_ragebot.h"
#include "../hacks/aimbot_recode.h"
#include "c_client.h"
#include "../hacks/c_prediction_system.h"
#include "../sdk/c_input.h"
#include "../hacks/c_antiaim.h"
#include "../sdk/c_client_state.h"
#include "../sdk/c_prediction.h"
#include "../sdk/c_base_view_model.h"
#include "../hacks/c_legitbot_beta.h"
#include "c_client_state_.h"
#include "c_events.h"
#include "c_cl_camera_height_restriction_debug.h"
#include "../LuaAPI.h"


void c_client::hook()
{
	static c_hook<c_base_client> hook(base_client());

	_shutdown = hook.apply<shutdown_t>(4, shutdown);
	_level_init_pre_entity = hook.apply<level_init_pre_entity_t>(5, level_init_pre_entity);
	_create_move = hook.apply<create_move_t>(22, ::create_move);
	_frame_stage_notify = hook.apply<frame_stage_notify_t>(37, frame_stage_notify);
}

void __fastcall c_client::shutdown(c_base_client* client, uint32_t)
{

	/*
#ifdef RELEASE
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	std::pair<uint32_t, uint32_t> result;

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
		{
			std::string name = entry.szExeFile;
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			if (fnv1a_rt(name.c_str()) == fnv1a("csgo.exe"))
			{
				if (entry.cntThreads <= 1)
				{
					const auto process = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
					if (process)
					{
						TerminateProcess(process, 0);
						CloseHandle(process);
					}
				}
			}
		}

	CloseHandle(snapshot);
#endif

	return _shutdown(client);
	*/
}
void c_client::level_init_pre_entity(c_base_client* client, uint32_t, const char* map_name)
{
	//c_net_channel_::apply_to_net_chan(client_state->net_channel);
	
	_level_init_pre_entity(client, map_name);
}

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**> (sig("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(sig("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}

template<typename FuncType>
__forceinline static FuncType CallVFuckingFunc(void* ppClass, int index)
{
	int* pVTable = *(int**)ppClass;
	int dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}


c_user_cmd *c_input::GetUserCmd(int nSlot, int sequence_number)
{
	typedef c_user_cmd*(__thiscall *GetUserCmd_t)(void*, int, int);
	return CallVFuckingFunc<GetUserCmd_t>(this, 8)(this, nSlot, sequence_number);
}

void __stdcall c_client::create_move(int sequence_number, float input_sample_frametime, bool active, bool& sendpacket)
{
	static const auto get_check_sum = reinterpret_cast<uint32_t(__thiscall*)(c_user_cmd*)>(sig("client_panorama.dll", "53 8B D9 83 C8"));
	
	// removed by laser
	/*static const auto engine_no_focus_sleep = cvar()->find_var(_("engine_no_focus_sleep"));

	engine_no_focus_sleep->set_value(0);*/

	// added by laser
	if (!get_check_sum) {
		cvar()->console_color_printf(false, c_color(255, 100, 100), "FAILED TO FIND `get_check_sum`\n");
	}


	_create_move(base_client(), sequence_number, input_sample_frametime, active);

	auto cmd = input->GetUserCmd(0, sequence_number);
	auto verified_cmd = &input->verified_commands[sequence_number % 150];
	if(!cmd || cmd == 0) return _create_move(base_client(), sequence_number, input_sample_frametime, active);
	
	const auto local = (c_cs_player*)client_entity_list()->get_client_entity(engine_client()->get_local_player());

	c_cl_camera_height_restriction_debug::in_cm = true;

	if (!cmd->command_number || !local || !local->is_alive() || !local->get_current_weapon_handle())
	{
		c_cl_camera_height_restriction_debug::in_cm = false;
		return;
	}

	if (c_net_channel_::hk == nullptr && client_state && client_state->net_channel)
		c_net_channel_::apply_to_net_chan(client_state->net_channel);
	
	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	c_base_combat_weapon::weapon_data* wpn_info = nullptr;

	if (!weapon || !(wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition())))
	{
		c_cl_camera_height_restriction_debug::in_cm = false;
		return;
	}

	animation_system->in_jump = cmd->buttons & c_user_cmd::jump;

	c_miscellaneous::unlock_client_command_buffer();

	g_nadepred.trace(cmd, local);

	//local_changers();
	cmd->buttons |= c_user_cmd::bull_rush;
	
	if (!config.rage.enabled)
	{
		prediction_system->initial(local, cmd, sequence_number);
		//c_legitbot::backtrack(local, cmd);
		c_legitbot_beta::aim(cmd);

		if (local->is_shooting(cmd, global_vars_base->curtime))
		{
			// store shot info.
			resolver::shot shot{};
			shot.start = local->get_shoot_position();
			shot.time = global_vars_base->curtime;
			shot.manual = true;
			shot.tickcount = cmd->tick_count;
		

			switch (config.rage.resolver) {
			case 1: c_resolver::register_shot(std::move(shot));
				break;
			case 2: c_resolver_beta::register_shot(std::move(shot));
				break;
			}
		}

		LUASYS->ExecuteCreateMove(cmd);

		animation_system->update_simple_local_player(local, cmd);
		prediction_system->restore();

		// verify new command.
		math::ensure_bounds(cmd->viewangles, *reinterpret_cast<c_vector3d*>(&cmd->forwardmove));
		verified_cmd->cmd = *cmd;
		verified_cmd->crc = get_check_sum(&verified_cmd->cmd);
		c_cl_camera_height_restriction_debug::in_cm = false;
		return;
	}

	cmd->buttons &= ~c_user_cmd::speed;//why?

	c_movement::run(local, cmd);

	prediction_system->initial(local, cmd, sequence_number);

	auto target_angle = cmd->viewangles;

	// initialize revolver.
	local->can_shoot(cmd, global_vars_base->curtime);

	antiaim->fakelag(local, cmd, sendpacket);

	const auto stopped_last_interval = c_ragebot::get_autostop_info(cmd).did_stop;

	c_ragebot::autostop(local, cmd);

	//if (!client_state->choked_commands && local->is_shooting(cmd, global_vars_base->curtime))
	//{
	//	cmd->buttons &= ~c_user_cmd::attack;
	//}
	//else if (local->is_shooting(cmd, global_vars_base->curtime))
	//{
	//	
	//		
	//}
	//else if (client_state->choked_commands && !local->is_shooting(cmd, global_vars_base->curtime))
	//{
	//	//c_aimbot::aimbot_run(local, cmd);
	//	
	//}
	auto backupbuttons = cmd->buttons;
	c_ragebot::aim(local, cmd, sendpacket);// run ragebot constantly cuz why not
	//auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()
	//	->get_client_entity_from_handle(local->get_current_weapon_handle()));

	


	if (local->is_shooting(cmd, global_vars_base->curtime))
	{
		c_aimhelper::fix_movement(cmd, target_angle);
		antiaim->shot_cmd = cmd->command_number;
		if(config.rage.fakelag_settings.disable_on_shooting)
			sendpacket =  true;//antiaim->is_fakeducking ? false : //i dont know why but we should really unchoke here


		/*this is already being done by the ragebot itself, only stores manual shots, why would we consider manual shots*/

		//// store shot info.
		//resolver::shot shot{};
		//shot.start = local->get_shoot_position();
		//shot.time = global_vars_base->curtime;
		//shot.manual = true;
		//shot.tickcount = cmd->tick_count;

		//switch (config.rage.resolver) {
		//case 1: c_resolver::register_shot(std::move(shot));
		//	break;
		//case 2: c_resolver_beta::register_shot(std::move(shot));
		//	break;
		//}
	}


	if ((!client_state->choked_commands && stopped_last_interval) || (stopped_last_interval && c_cs_player::can_not_shoot_due_to_cock && weapon->get_item_definition() == weapon_revolver))
		c_ragebot::autostop(local, cmd);

	if (!c_ragebot::get_autostop_info(cmd).did_stop)
	{
		cmd->forwardmove = prediction_system->original_cmd.forwardmove;
		cmd->sidemove = prediction_system->original_cmd.sidemove;
		prediction_system->repredict(local, cmd);
	}

	

	static const auto recoil_scale = cvar()->find_var(_("weapon_recoil_scale"));

	cmd->viewangles -= local->get_punch_angle() * recoil_scale->get_float();

	if (cmd->buttons & c_user_cmd::attack && antiaim->is_fakeducking)
	{
	}
	else
		antiaim->run(local, cmd, sendpacket, sequence_number);

	if (sendpacket)
	{
		antiaim->is_lby_broken = fabsf(math::normalize_yaw(math::normalize_yaw(local->get_lby()) - math::normalize_yaw(cmd->viewangles.y))) <= 2 * 30.0f;
	}

	if (config.chams.local.enabled && !sendpacket && ~c_user_cmd::use && config.rage.fakelag_settings.enabled)
	{
		if (local->is_alive())
		{
			c_qangle backup_angles = local->get_abs_angles();
			local->set_abs_angles(c_vector3d(backup_angles.x, antiaim->last_fake, backup_angles.z));

			local->get_most_recent_model_bone_counter() = 0;
			local->get_last_bone_setup_time() = -FLT_MAX;

			local->setup_bones(antiaim->last_fake_matrix, 0x100, bone_used_by_server, global_vars_base->curtime);
			local->set_abs_angles(backup_angles);

			local->get_most_recent_model_bone_counter() = 0;
			local->get_last_bone_setup_time() = -FLT_MAX;
		}
	}

	if (config.misc.indicators && config.misc.indicator_antiaim && config.misc.indicator_antiaim_l)
	{
		c_vector2d src, dst;
		c_vector3d dst3D, src3D, forward;

		game_trace tr;
		ray r;
		c_trace_filter filter;

		filter.skip_entity = local;

		/*LBY_ANGLE*/
		math::angle_vectors(c_qangle(0, local->get_lby(), 0), forward);
		src3D = local->get_origin();
		dst3D = src3D + (forward * 50.f);

		r.init(src3D, dst3D);

		engine_trace()->trace_ray(r, 0, &filter, &tr);
		debug_overlay()->add_line_overlay(src3D, tr.endpos, 0, 255, 0, 1, global_vars_base->interval_per_tick * 2);

		

		/*REAL_ANGLE*/
		math::angle_vectors(c_qangle(0, antiaim->last_real, 0), forward);
		src3D = local->get_origin();
		dst3D = src3D + (forward * 50.f);

		r.init(src3D, dst3D);

		engine_trace()->trace_ray(r, 0, &filter, &tr);

		debug_overlay()->add_line_overlay(src3D, tr.endpos, 0, 255, 255, 1, global_vars_base->interval_per_tick * 2);
		

		/*DESYNC_ANGLE */
		math::angle_vectors(c_qangle(0, antiaim->last_fake, 0), forward);
		src3D = local->get_origin();
		dst3D = src3D + (forward * 50.f);

		r.init(src3D, dst3D);

		engine_trace()->trace_ray(r, 0, &filter, &tr);
		debug_overlay()->add_line_overlay(src3D, tr.endpos, 255, 0, 0, 1, global_vars_base->interval_per_tick * 2);
		
	}
	
	prediction_system->repredict(local, cmd);
	prediction_system->restore();

	// remove duck stamina.
	c_miscellaneous::pushscale();
	c_miscellaneous::gravity();
	c_miscellaneous::Instastop(cmd);
	c_miscellaneous::remove_duck_stamina(cmd);

	

	if (antiaim->is_fakeducking)
	{
		if (client_state->choked_commands > 6)
			cmd->buttons |= c_user_cmd::flags::duck;
		else
			cmd->buttons &= ~c_user_cmd::flags::duck;
	}

	// verify new command.
	c_aimhelper::fix_movement(cmd, target_angle);
	math::ensure_bounds(cmd->viewangles, *reinterpret_cast<c_vector3d*>(&cmd->forwardmove));

	/* lua insertion right here */
	LUASYS->ExecuteCreateMove(cmd);

	c_miscellaneous::set_buttons_for_direction(cmd);
	verified_cmd->cmd = *cmd;
	verified_cmd->crc = get_check_sum(&verified_cmd->cmd);
	c_cl_camera_height_restriction_debug::in_cm = false;
}

//convar* r_DrawFullBright = nullptr;

bool ReallocatedDeathNoticeHUD = false;

void c_client::frame_stage_notify(c_base_client* client, uint32_t, const clientframestage stage)
{
	static int old_sky;
	static bool old_night_mode;
	static std::string old_sky_name = "";

	if (!engine_client()->is_ingame())
	{
		old_sky = 0;
		old_sky_name = "";
		old_night_mode = false;
		if (!engine_client()->is_ingame()) {
			if (c_net_channel_::hk) {
				c_net_channel_::hk->unapply(46);
				c_net_channel_::_send_datagram = nullptr;
				c_net_channel_::hk = nullptr;
			}
		}

		return _frame_stage_notify(client, stage);
	}

	const auto local = c_cs_player::get_local_player();

	static auto cycle = 0.f;
	static auto anim_time = 0.f;

	const auto view_model = local ? reinterpret_cast<c_base_view_model*>(client_entity_list()->get_client_entity_from_handle(local->get_view_model())) : nullptr;
	
	if(stage == clientframestage::frame_net_update_postdataupdate_start && engine_client()->is_ingame() && local)
		c_miscellaneous::clantag();

	if (engine_client()->is_ingame() && local)
	
	{
		if (cvar()) {//nullptr check
			static const auto sv_showimpacts = cvar()->find_var(_("sv_showimpacts"));
			static const auto sv_showimpacts_penetration = cvar()->find_var(_("sv_showimpacts_penetration"));
			static const auto r_DrawFullBright = cvar()->find_var(_("mat_fullbright"));
			if (config.misc.sv_impacts)
			{
				if (sv_showimpacts && local)
				{		
					sv_showimpacts->set_value(1);
				}

				if (sv_showimpacts_penetration && local)
					sv_showimpacts_penetration->set_value(1);
			}
			else {
				if (sv_showimpacts && local)
				{
					sv_showimpacts->set_value(0);
				}

				if (sv_showimpacts_penetration && local)
					sv_showimpacts_penetration->set_value(0);
			}
			if (r_DrawFullBright) {
				if (config.misc.full_bright)
					r_DrawFullBright->set_value(1);
				else
					r_DrawFullBright->set_value(0);
			}
		}
			/*
			static const auto r_dlightsenable = cvar()->find_var(_("r_dlightsenable"));
				r_dlightsenable->set_value(1);
			*/

		static std::string old_Skyname = "";
		static bool OldNightmode = false;
		static c_vector3d olddata;
		static int OldSky;
		if (!engine_client()->is_connected() || !engine_client()->is_ingame() || !local /*|| !local->is_alive()*/)
		{
			old_Skyname = "";
			OldNightmode = false;
			OldSky = 0;

		}
		else
		{
			static convar* r_DrawSpecificStaticProp;
			static float changer_shit = 7.5f;

			c_vector3d shit = c_vector3d(//i love the naming?!
				changer_shit / float(config.misc.nightmode_color.red),
				changer_shit / float(config.misc.nightmode_color.green),
				changer_shit / float(config.misc.nightmode_color.blue));

			if (OldNightmode != config.misc.nightmode || (shit.x != olddata.x || shit.y != olddata.y || shit.z != olddata.z))
			{
				static const auto sv_skyname = cvar()->find_var(_("sv_skyname"));
				r_DrawSpecificStaticProp = cvar()->find_var(_("r_DrawSpecificStaticProp"));
				r_DrawSpecificStaticProp->set_value(0);
				for (MaterialHandle_t i = material_system()->FirstMaterial(); i != material_system()->InvalidMaterial(); i = material_system()->NextMaterial(i))
				{
					IMaterial* pMaterial = material_system()->GetMaterial(i);
					if (!pMaterial)
						continue;
					if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
					{
						if (config.misc.nightmode) {
							sv_skyname->set_value(_("sky_csgo_night02"));

							if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
								pMaterial->ColorModulate(shit.x, shit.y, shit.z);
							else
								pMaterial->ColorModulate(shit.x, shit.y, shit.z);
						}
						else {
							sv_skyname->set_value(_("sky_cs15_daylight04_hdr"));
							pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
						}
					}
				}
				OldNightmode = config.misc.nightmode;
				olddata = shit;
			}
		}
		
	}

	if (stage == frame_render_start)
	{
		const auto local = c_cs_player::get_local_player();
	
		if (local && local->is_alive())
		{

			switch (config.rage.resolver) {
			case 1: c_resolver::on_render_start();
				break;
			case 2: c_resolver_beta::on_render_start();
				break;
			}

			
				

			if (local && local->is_alive())
				local->set_abs_angles(animation_system->local_animation.abs_ang);

			if (config.misc.preserve_feed)
			{
				static void(__thiscall *ClearDeathNotices)(DWORD);
				static DWORD* deathNotice;

				if (!ReallocatedDeathNoticeHUD)
				{
					//Find HUD cuz we haven't found it yet
					ReallocatedDeathNoticeHUD = true;
					deathNotice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");
					ClearDeathNotices = (void(__thiscall*)(DWORD))(sig("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58"));
				}
				else
				{
					if (deathNotice)
					{
						//Preserve killfeed
						if (c_events::round_flags == ROUND_IN_PROGRESS)
						{
							float* localDeathNotice = (float*)((DWORD)deathNotice + 0x50);
							if (localDeathNotice)
							{
								*localDeathNotice = FLT_MAX;
							}
						}

						//Clear killfeed cuz round is done
						if (c_events::round_flags == ROUND_STARTING && deathNotice - 20)
						{
							if (ClearDeathNotices)
							{
								ClearDeathNotices(((DWORD)deathNotice - 20));
							}
						}
					}
				}
			}
		}
		else {
			ReallocatedDeathNoticeHUD = false;
		}
	}

	_frame_stage_notify(client, stage);

	if (stage == frame_render_start)
	{
		antiaim->increment_visual_progress();

		c_esp::store_data();
		//c_miscellaneous::set_viewmodel_parameters();//what is that?
		c_miscellaneous::remove_flash();
		c_miscellaneous::remove_smoke();
		c_miscellaneous::engine_radar();
		c_miscellaneous::aspectratio();
	}
	
	if ( stage == frame_net_update_postdataupdate_start && local && local->is_alive())
	{
		
		static bool der = 0;//crashes if on
		if(der){
			const auto& info = recoil_info[local->get_tick_base() & 63];
			if (sizeof(info) > 0)
			{
				if (fabsf(info.aim_punch.y - local->get_punch_angle().y) < 0.5f
					&& fabsf(info.aim_punch.x - local->get_punch_angle().x) < 0.5f
					&& fabsf(info.aim_punch.z - local->get_punch_angle().z) < 0.5f)
					local->get_punch_angle() = info.aim_punch;

				if (fabsf(info.aim_punch_vel.y - local->get_punch_angle_vel().y) < 0.5f &&
					fabsf(info.aim_punch_vel.x - local->get_punch_angle_vel().x) < 0.5f &&
					fabsf(info.aim_punch_vel.z - local->get_punch_angle_vel().z) < 0.5f)
					local->get_punch_angle_vel() = info.aim_punch_vel;

				if (fabsf(info.view_offset.y - local->get_view_offset().y) < 0.5f &&
					fabsf(info.view_offset.x - local->get_view_offset().x) < 0.5f &&
					fabsf(info.view_offset.z - local->get_view_offset().z) < 0.5f)
					local->get_view_offset() = info.view_offset;
			}
		}
		if (der) {
			if (input && view_model && !input->camera_in_third_person)
			{
				view_model->get_anim_time() = anim_time;
				view_model->get_cycle() = cycle;
			}

			
		}animation_system->server_layers = *local->get_animation_layers();
	}
	
	/*if (view_model)
	{
		cycle = view_model->get_cycle();
		anim_time = view_model->get_anim_time();
	}*/

	if (stage == frame_net_update_end)
		animation_system->post_player_update();
}

__declspec(naked) void create_move(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		push ebx
		push esp
		push[esp + 0x14]
		push[esp + 0x14]
		push[esp + 0x14]
		call c_client::create_move
		pop ebx
		ret 0x0c
	}
}
