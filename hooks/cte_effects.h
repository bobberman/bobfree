#pragma once

#include "c_client_state_.h"
#include "../sdk/c_client_entity_list.h"
#include "../utils/c_hook.h"
#include "../utils/c_config.h"
#include "../sdk/c_cvar.h"
#include "../sdk/c_game_rules.h"

class C_TEFireBullets
{
public:
	char pad[12];
	int		m_iPlayer; //12
	int _m_iItemDefinitionIndex;
	c_vector3d	_m_vecOrigin;
	c_qangle	m_vecAngles;
	int		_m_iWeaponID;
	int		m_iMode;
	int		m_iSeed;
	float	m_flSpread;
};

enum DataUpdateType_t
{
	DATA_UPDATE_CREATED = 0,
	//	DATA_UPDATE_ENTERED_PVS,
	DATA_UPDATE_DATATABLE_CHANGED
	//	DATA_UPDATE_LEFT_PVS,
	//DATA_UPDATE_DESTROYED,
};

class c_ct_effects
{
	typedef void(__thiscall* FireBullets_t)(C_TEFireBullets*, DataUpdateType_t);

public:
	static void hook();
private:
	inline static FireBullets_t _FireBullets;

	static void __stdcall TEFireBulletsPostDataUpdate_h(DataUpdateType_t updateType);
	static void __stdcall FireBullets_PostDataUpdate(C_TEFireBullets *thisptr, DataUpdateType_t updateType);

};
