#include "cte_effects.h"
#include "../hacks/c_resolver.h"
#include "../sdk/c_debug_overlay.h"

c_fire g_firingresolver;

void c_ct_effects::hook()
{
	auto dwFireBullets = *(DWORD**)(sig("client_panorama.dll", "55 8B EC 51 53 56 8B F1 BB ? ? ? ? B8") + 0x131);

	static c_hook hook((PDWORD)dwFireBullets);
	_FireBullets = hook.apply<FireBullets_t>(7, TEFireBulletsPostDataUpdate_h);
}

void __stdcall c_ct_effects::FireBullets_PostDataUpdate(C_TEFireBullets *thisptr, DataUpdateType_t updateType)
{
	const auto local = c_cs_player::get_local_player();

	if (!engine_client()->is_ingame() || !engine_client()->is_connected())
		return _FireBullets(thisptr, updateType);

	if (config.rage.enabled && thisptr && config.esp.show_on_shot_hitboxes)
	{
		int iPlayer = thisptr->m_iPlayer + 1;

		if (iPlayer < 64)
		{
			auto player = (c_cs_player*)client_entity_list()->get_client_entity(iPlayer);

			if (!player->is_enemy() || !player->is_alive() || player->get_gun_game_immunity())
				return _FireBullets(thisptr, updateType);

			matrix3x4 animation_matrix[128];
			matrix3x4 setup_bones_matrix[128];

			auto animations = animation_system->get_latest_firing_animation(player);

			if (player->setup_bones(setup_bones_matrix, 128, bone_used_by_anything, player->get_simtime()))
			{
				const auto model = player->get_model();

				if (!model)
					return _FireBullets(thisptr, updateType);

				const auto hdr = model_info_client()->get_studio_model(model);

				if (!hdr)
					return _FireBullets(thisptr, updateType);

				if (!animations.has_value())
					return _FireBullets(thisptr, updateType);

				std::memcpy(animation_matrix, animations.value()->bones, sizeof(animations.value()->bones));

				const auto set = hdr->get_hitbox_set(0);

				if (set)
				{
					for (auto i = 0; i < set->numhitboxes; i++)
					{
						const auto hitbox = set->get_hitbox(i);

						if (!hitbox)
							continue;

						if (hitbox->radius == -1.0f)
						{
							const auto position = math::matrix_position(animation_matrix[hitbox->bone]);
							const auto position_actual = math::matrix_position(setup_bones_matrix[hitbox->bone]);

							const auto roation = math::angle_matrix(hitbox->rotation);

							auto transform = math::multiply_matrix(animation_matrix[hitbox->bone], roation);
							auto transform_actual = math::multiply_matrix(setup_bones_matrix[hitbox->bone], roation);

							const auto angles = math::matrix_angles(transform);
							const auto angles_actual = math::matrix_angles(transform_actual);

							debug_overlay()->add_box_overlay(position, hitbox->bbmin, hitbox->bbmax, angles, 255, 0, 0, 150, 0.8f);
							debug_overlay()->add_box_overlay(position_actual, hitbox->bbmin, hitbox->bbmax, angles_actual, 0, 0, 255, 150, 0.8f);
						}
						else
						{
							c_vector3d min, max, min_actual, max_actual;

							math::vector_transform(hitbox->bbmin, animation_matrix[hitbox->bone], min);
							math::vector_transform(hitbox->bbmax, animation_matrix[hitbox->bone], max);

							math::vector_transform(hitbox->bbmin, setup_bones_matrix[hitbox->bone], min_actual);
							math::vector_transform(hitbox->bbmax, setup_bones_matrix[hitbox->bone], max_actual);

							debug_overlay()->add_capsule_overlay(min, max, hitbox->radius, 255, 0, 0, 150, 0.8f);
							debug_overlay()->add_capsule_overlay(min_actual, max_actual, hitbox->radius, 0, 0, 255, 150, 0.8f);
						}
					}
				}
			}
		}
	}

	_FireBullets(thisptr, updateType);

}

__declspec (naked) void __stdcall c_ct_effects::TEFireBulletsPostDataUpdate_h(DataUpdateType_t updateType)
{
	__asm
	{
		push[esp + 4]
		push ecx
		call FireBullets_PostDataUpdate
		retn 4
	}
}