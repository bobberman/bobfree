#pragma once
#include "includes.h"
#include "renderer/c_renderer.h"
#include "hooks/idirect3ddevice9.h"




struct groupbox_info {
	float x;
	float y;
	float w;
	float h;
	const char* title;
};

enum previous_control {
	control_invalid = -1,
	control_checkbox = 0,
	control_slider,
	control_combobox,
	control_multicombo,
	control_color
};



class framework : public c_singleton<framework> {
public:

	static bool run();
	static int tabs(std::vector<std::string> tabs);
	static bool groupbox(const char * title, int size, float scrollamount = 0);

	static void end_groupbox();
	static bool checkbox(const char * title, bool * item);
	static float slider(const char * title, float * item, float min, float max, const char * suffix = "", bool hidebar = false);
	static int slider(const char * title, int * item, int min, int max, const char * suffix = "", bool hidebar = false);
	static int combo(const char * title, int * item, std::vector<std::string> items);
	static void run_combo();
	static void multicombo(const char * title, bool item[], std::vector<std::string> items);
	static void run_multicombo();

	static void next_column();
	static c_color colors(c_color * item);
	static void run_color();
	static void run_color_dropdown();
	static void run_color_drag();

	static void hotkey(const char * title, int * item);

	static void text(const char * title);
	

private:

	static groupbox_info get_info();
	static void setup();
	static void handle_movement();
	static c_color handle_color(c_color c, int _max = 255);

	/* RENDERING */
	static 	void box(int x, int y, int w, int h, c_color c, int max = 255);
	static void outline(int x, int y, int w, int h, c_color c, int max = 255);
	static void text(int x, int y, const char * string, c_color c, bool center = false, int max = 255, unsigned long font = fnv1a("menu_controls"));
	static c_vector2d text_size(const char * text, unsigned long font = fnv1a("menu_controls"));
	static void line(int x, int y, int x2, int y2, c_color c, int max = 255);


	static void handle_keys();
	static c_vector2d _mouseposition();
	static bool _inparam(int x, int y, int x2, int y2);
	static float lerp(float t, float a, float b);

	// main
	

	

};







