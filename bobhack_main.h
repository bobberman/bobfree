#pragma once

#include "includes.h"

struct rifk_loader_info
{
	// not null terminated - use with caution
	char loader_path[MAX_PATH];
	char username[32];
	char hwid[32];
	int days_remaining;
	DWORD tickcount;
	bool is_beta_injected;
};

extern rifk_loader_info g_loader_info;
extern DWORD g_image_base;

class c_rifk : public c_singleton<c_rifk>
{
public:
	c_rifk();
};

#define rifk c_rifk::instance()
