#include "c_chams.h"
#include "../security/string_obfuscation.h"
#include "../sdk/c_material_system.h"
#include "../sdk/c_render_view.h"
#include "../menu/framework/macros.h"
#include "../utils/c_config.h"
#include "../sdk/c_client_entity_list.h"
#include "c_animation_system.h"
#include "../utils/math.h"
#include "../sdk/c_client_state.h"
#include "../sdk/c_prediction.h"

c_chams::c_chams() : current_player(nullptr), current_matrix(nullptr), second_pass(false), alpha(255), direction(false) { }

float timer;




void c_chams::latch_timer()
{
	if (!engine_client()->is_ingame())
		return;

	if (alpha == 0 || alpha == 255)
		direction = !direction;

	linear_fade(alpha, 0, 255, 255 / 2.5f, direction);
}

void c_chams::draw_players()
{
	client_entity_list()->for_each_player_fixed_z_order([&](c_cs_player* player) -> void
	{
		if (player) {
			instance()->current_player = player;
			instance()->second_pass = false;

			const auto conf = config.chams.enemy;

			if (conf.xqz)
			{
				instance()->second_pass = true;
				player->draw_model(0x1, 255);
			}
		}
	});

	instance()->current_player = nullptr;

	const auto local = c_cs_player::get_local_player();

	if (local && local->is_alive())
	{
		instance()->second_pass = true;
		instance()->current_player = local;
		local->draw_model(0x1, 255);
	}

	instance()->second_pass = false;
	instance()->current_player = nullptr;
}
bool c_chams::get_simple_backtrack_matrix(c_cs_player* player, matrix3x4* out)
{
	if (!engine_client()->is_ingame())
		return false;

	//auto range = 1;//config.legit.backtrack / 100.f;

	//if (config.rage.enabled)
		//range = 1.f;

	const auto last = animation_system->get_oldest_animation(player);

	if (!last || !last.has_value())
		return false;

	memcpy(out, last.value()->bones, sizeof(matrix3x4[128]));
}
bool c_chams::get_backtrack_matrix(c_cs_player* player, matrix3x4* out)
{
	if (!engine_client()->is_ingame())
		return false;


	auto range = 1;//config.legit.backtrack / 100.f;

	if (config.rage.enabled)
		range = 1.f;

	const auto last = animation_system->get_intermediate_animations(player);

	if (!last || !last.has_value())
		return false;

	const auto& first_invalid = last.value().first;
	const auto& last_valid = last.value().second;

	if ((first_invalid->origin - player->get_abs_origin()).length() < 7.5f)
		return false;

	if (first_invalid->dormant)
		return false;

	if (last_valid->sim_time - first_invalid->sim_time > 0.5f)
		return false;

	const auto next = last_valid->origin;
	if (!prediction() || !prediction()->get_unpredicted_globals())return false;// nullptr 
	const auto curtime = prediction()->get_unpredicted_globals()->curtime;

	auto delta = 1.f - (curtime - last_valid->interp_time) / (last_valid->sim_time - first_invalid->sim_time);
	if (delta < 0.f || delta > 1.f)
		last_valid->interp_time = curtime;

	delta = 1.f - (curtime - last_valid->interp_time) / (last_valid->sim_time - first_invalid->sim_time);

	const auto lerp = math::interpolate(next, first_invalid->origin, std::clamp(delta, 0.f, 1.f));

	matrix3x4 ret[128];
	memcpy(ret, first_invalid->bones, sizeof(matrix3x4[128]));
	
	for (size_t i{}; i < 128; ++i)
	{
		const auto matrix_delta = math::matrix_get_origin(first_invalid->bones[i]) - first_invalid->origin;
		math::matrix_set_origin(matrix_delta + lerp, ret[i]);
	}

	memcpy(out, ret, sizeof(matrix3x4[128]));
	return true;
}


bool loaded_chams = false;
c_material* c_chams::GetMat(int material) {
	if (!loaded_chams) {
		std::ofstream("csgo\\materials\\textured_virt.vmt") << R"("VertexLitGeneric"
{
	"$basetexture"	"vgui/white"
	"$model"		"1"
	"$flat"			"0"
	"$nocull"		"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$ignorez"		"0"
	"$znearer"		"0"
	"$wireframe"	"0"
})";
		std::ofstream("csgo\\materials\\custom_virt.vmt") << R"("VertexLitGeneric" 
{
	"$basetexture"	"vgui/white"
	"$envmap"       "env_cubemap"
	"$model"		"1"
	"$flat"			"0"
	"$nocull"		"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$ignorez"		"0"
	"$znearer"		"0"
	"$wireframe"	"0"
})";
		std::ofstream("csgo\\materials\\flat_virt.vmt") << R"("UnlitGeneric"
{
	"$basetexture"	"vgui/white"
	"$model"		"1"
	"$flat"			"1"
	"$nocull"		"1"
	"$selfillum"	"1"
	"$halflambert"	"1"
	"$nofog"		"1"
	"$ignorez"		"0"
	"$znearer"		"0"
	"$wireframe"	"0"
})";
		loaded_chams = true;
	}


	switch (material) {
	case 0: default: return find_mat("textured_virt");
	case 1: return find_mat("flat_virt");
	case 2: return find_mat("custom_virt");
	case 3: return find_mat("dev/glow_armsrace");
	case 4: return find_mat("models/inventory_items/dogtags/dogtags_lightray");
	}
}

c_material* c_chams::GetOverlay(int material) {


	switch (material) {
	case 0: default: return find_mat("textured_virt");
	case 1: return find_mat("flat_virt");
	case 2: return find_mat("custom_virt");
	case 3: return find_mat("dev/glow_armsrace");
	case 4: return find_mat("models/inventory_items/dogtags/dogtags_lightray");
	}
}

float alphas = 100.f;
bool alpha_direction = false;


void c_chams::CustomMaterial(c_material* mat) {
	
	

	mat->set_material_var_flag(material_var_wireframe, config.chams.option1);
	if (config.chams.option2) {
		render_view()->set_blend(instance()->alpha / 255.f);
	}
}


void c_chams::player_chams(const std::function<void()> original, c_config::config_conf::chams_conf::chams& conf, bool draw_fake, bool scope_blend)
{
	if (!conf.enabled && !draw_fake)
		return;

	if (!conf.desync_type > 0 && draw_fake) return;


	int type = draw_fake ? conf.desync_type - 1 : conf.type;
	c_color c = draw_fake ? conf.desync_color : conf.color;
	bool preserve = draw_fake ? conf.option2 : conf.option1;


	if (type == 5)
	{
		const auto alpha = config.chams.option2 ? instance()->alpha : c.alpha;
		if (config.chams.option3 ) {
			render_view()->set_blend((preserve ? 255 : alpha) / 255.f);
			original();
		}
			

		
		auto mat = GetMat(config.chams.base);
		
		mat->incrementreferencecount();
		mat->set_material_var_flag(material_var_wireframe, config.chams.option1);
		set_ignorez(instance()->second_pass ? conf.xqz : false, mat);
		modulate(c, mat);

		modulate_exp(mat, scope_blend ? std::clamp(alpha / 4, 0, 255) : alpha / 255.f);
		render_view()->set_blend(alpha / 255.f);
		model_render()->forced_material_override(mat);
		original();
		if (instance()->second_pass && conf.xqz)
		{
			mat->incrementreferencecount();
			if (mat != nullptr) {
				set_ignorez(false, mat);
				modulate(conf.xqz_color, mat);
				render_view()->set_blend(scope_blend ? std::clamp(alpha / 4, 0, 255) : alpha / 255.f);
				model_render()->forced_material_override(mat);
				original();
			}
		}
	}
	else
	{
		const auto alpha = c.alpha;
	
		auto mat = GetMat(type);	
		mat->incrementreferencecount();
		mat->set_material_var_flag(material_var_wireframe, false);
		modulate(c, mat);
		set_ignorez(instance()->second_pass ? conf.xqz : false, mat);
		modulate_exp(mat, scope_blend ? std::clamp(alpha / 4, 0, 255) : alpha / 255.f);
		render_view()->set_blend(alpha / 255.f);
		model_render()->forced_material_override(mat);
		original();
		if (instance()->second_pass && conf.xqz)
		{
			mat->incrementreferencecount();
			if (mat != nullptr) {
				set_ignorez(false, mat);
				modulate(conf.xqz_color, mat);
				render_view()->set_blend(scope_blend ? std::clamp(alpha / 4, 0, 255) : alpha / 255.f);
				model_render()->forced_material_override(mat);
				original();
			}
		}
	}

	

}

void c_chams::hand_chams(const std::function<void()> original, c_config::config_conf::chams_conf::chams& conf, bool draw_hand)
{
	if (!conf.enabled) return;
	int type = conf.type;
	c_color c = conf.color;

	if (type == 5)
	{
		const auto alpha = config.chams.option2 ? instance()->alpha : c.alpha;
		if (config.chams.option3) {
			render_view()->set_blend((conf.option2 ? 255 : alpha) / 255.f);
			original();
		}



		auto mat = GetMat(config.chams.base);

		mat->incrementreferencecount();
		mat->set_material_var_flag(material_var_wireframe, config.chams.option1);
		set_ignorez(instance()->second_pass ? conf.xqz : false, mat);
		modulate(c, mat);
		modulate_exp(mat, alpha / 255.f);
		render_view()->set_blend(alpha / 255.f);
		model_render()->forced_material_override(mat);

		original();
		if (instance()->second_pass && conf.xqz)
		{
			if (mat != nullptr) {
				set_ignorez(false, mat);
				modulate(conf.xqz_color, mat);
				render_view()->set_blend(alpha / 255.f);
				model_render()->forced_material_override(mat);
				original();
			}
		}
	}
	else
	{
		const auto alpha = c.alpha;

		auto mat = GetMat(type);
		mat->incrementreferencecount();
		mat->set_material_var_flag(material_var_wireframe, false);
		modulate(c, mat);
		set_ignorez(instance()->second_pass ? conf.xqz : false, mat);
		modulate_exp(mat, alpha / 255.f);
		render_view()->set_blend(alpha / 255.f);
		model_render()->forced_material_override(mat);
		original();
		if (instance()->second_pass && conf.xqz)
		{
			if (mat != nullptr) {
				set_ignorez(false, mat);
				modulate(conf.xqz_color, mat);
				render_view()->set_blend(alpha / 255.f);
				model_render()->forced_material_override(mat);
				original();
			}
		}
	}
}


void c_chams::weapon_chams(const std::function<void()> original, c_config::config_conf::chams_conf::chams& conf, bool draw_weapon)
{
	if (!conf.enabled) return;
	int type =  conf.type;
	c_color c = conf.color;

	if (type == 5)
	{
		const auto alpha = config.chams.option2 ? instance()->alpha : c.alpha;
		if (config.chams.option3) {
			render_view()->set_blend((conf.option1 ? 255 : alpha) / 255.f);
			original();
		}



		auto mat = GetMat(config.chams.base);

		mat->incrementreferencecount();
		mat->set_material_var_flag(material_var_wireframe, config.chams.option1);
		set_ignorez(instance()->second_pass ? conf.xqz : false, mat);
		modulate(c, mat);
		modulate_exp(mat, alpha / 255.f);
		render_view()->set_blend(alpha / 255.f);
		model_render()->forced_material_override(mat);
		original();
		if (instance()->second_pass && conf.xqz)
		{
			if (mat != nullptr) {
				set_ignorez(false, mat);
				modulate(conf.xqz_color, mat);
				render_view()->set_blend(alpha / 255.f);
				model_render()->forced_material_override(mat);
				original();
			}
		}
	}
	else
	{
		const auto alpha = c.alpha;

		auto mat = GetMat(type);
		mat->incrementreferencecount();
		mat->set_material_var_flag(material_var_wireframe, false);
		modulate(c, mat);
		set_ignorez(instance()->second_pass ? conf.xqz : false, mat);
		modulate_exp(mat, alpha / 255.f);
		render_view()->set_blend(alpha / 255.f);
		model_render()->forced_material_override(mat);
		original();
		if (instance()->second_pass && conf.xqz)
		{
			if (mat != nullptr) {
				set_ignorez(false, mat);
				modulate(conf.xqz_color, mat);
				render_view()->set_blend(alpha / 255.f);
				model_render()->forced_material_override(mat);
				original();
			}
		}
	}
}

void c_chams::modulate(const c_color color, c_material* material)
{
	if (!engine_client()->is_ingame())
		return;

	if (material)
	{
		material->modulate(color);

		const auto tint = material->find_var(_("$envmaptint"));

		if (tint)
			tint->set_vector(c_vector3d(color.red / 255.f, color.green / 255.f, color.blue / 255.f));
	}

	render_view()->set_color_modulation(color);
}

void c_chams::modulate_exp(c_material* material, const float alpha, const float width)
{
	if (!engine_client()->is_ingame())
		return;

	if (!material)
		return;

	const auto transform = material->find_var(_("$envmapfresnelminmaxexp"));

	if (transform)
		transform->set_vector_component(width * alpha, 1);
}

void c_chams::set_ignorez(const bool enabled, c_material* mat)
{
	if (!engine_client()->is_ingame())
		return;

	mat->incrementreferencecount();

	mat->set_material_var_flag(material_var_ignorez, enabled);
}



