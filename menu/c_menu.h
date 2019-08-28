#pragma once

#include "../includes.h"

#include "framework/c_flow_layout.h"
 
class c_menu : public c_singleton<c_menu>, public c_flow_layout
{
public:
	
	c_menu();

	bool is_open() const;
	bool d3dinit = false;
	bool input_shouldListen = false;

	bool apply_nightmode = false;

	void GUI_Init(HWND, IDirect3DDevice9*);
	void watermark();

	float lerp(float t, float a, float b);

	void mainWindow();

private:

	LPDIRECT3DTEXTURE9 image;
	LPD3DXSPRITE sprite;

	void* image_data;
	uint32_t image_size;

	float scale;

	bool open;
	int alpha;

	struct pages {
		int main = 0;
		struct sub_pages {
			int page = 0;
		} sub_pages [69] /*ye know*/;
	} pages;

};



#define menu c_menu::instance()
