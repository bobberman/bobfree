#include "c_render_view_.h"
#include "../utils/c_hook.h"
#include "../hacks/c_chams.h"
#include "../sdk/c_glow_object_manager.h"

void c_render_view_::hook()
{
	static c_hook<c_render_view> hook(render_view());
	_scene_end = hook.apply<scene_end_t>(9, scene_end);
}

void ApplyGlow() //to_do
{
	auto local = c_cs_player::get_local_player();

	for (int i = 0; i < glow_object_manager->size; ++i)
	{
		c_glow_object_manager::GlowObjectDefinition_t* obj = &glow_object_manager->m_GlowObjectDefinitions[i];

		c_cs_player* objEntity = obj->GetEntity();

		if (obj->IsEmpty() || !objEntity)
			continue;

		if (!objEntity->is_alive() || objEntity->get_team() == local->get_team() || (objEntity == local))
			continue;

		if (objEntity->get_class_id() == 38)
		{
			obj->Set(c_color::gradient3());
		}
	}
}

void __fastcall c_render_view_::scene_end(c_render_view* render_view, uint32_t)
{
	_scene_end(render_view);

	if (engine_client()->is_ingame())
		c_chams::draw_players();
}
