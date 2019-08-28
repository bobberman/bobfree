#pragma once

#include "c_utlvector.h"
#include "c_cs_player.h"
#include "c_color.h"

class c_glow_object_manager
{
public:
	static c_glow_object_manager* get();

	class GlowObjectDefinition_t
	{
	public:
		void Set(c_color color)
		{
			m_vGlowColor = c_vector3d(255, 255, 25);
			m_flGlowAlpha = 255;
			m_bRenderWhenOccluded = true;
			m_bRenderWhenUnoccluded = false;
			m_flBloomAmount = 1.f;
			
		}

		c_cs_player* GetEntity()
		{
			return m_hEntity;
		}

		bool IsEmpty() const { return m_nNextFreeSlot != GlowObjectDefinition_t::ENTRY_IN_USE; }

	public:
		c_cs_player* m_hEntity;
		c_vector3d				m_vGlowColor;
		float				m_flGlowAlpha;

		char				unknown[4];
		float				flUnk;
		float				m_flBloomAmount;
		float				localplayeriszeropoint3;

		bool				m_bRenderWhenOccluded;
		bool				m_bRenderWhenUnoccluded;
		bool				m_bFullBloomRender;
		char				unknown1[1];

		int					m_nFullBloomStencilTestValue;
		int					iUnk;
		int					m_nSplitScreenSlot;
		int					m_nNextFreeSlot;

		static const int END_OF_FREE_LIST = -1;
		static const int ENTRY_IN_USE = -2;
	};

	GlowObjectDefinition_t* m_GlowObjectDefinitions;
	int		max_size;
	int		pad;
	int		size;
	GlowObjectDefinition_t* m_GlowObjectDefinitions2;
	int		currentObjects;
};

#define glow_object_manager c_glow_object_manager::get()
