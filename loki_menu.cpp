#include "loki_framework.h"


static float _alpha;
static c_vector2d _size;
static c_vector2d _bounds;
static c_vector2d _position;
static c_vector2d _window_size;
static c_vector2d _dragging_position;

static c_color _color_line;
static c_color _color_background;
static c_color _color_partial;
static c_color _color_group;
static c_color _color_text;

static c_color _color_element_back;
static c_color _color_element_outline;


static bool _dragging;
static bool _clicking;
static bool _lclicking;
static bool _ctrlclicking;
static bool _escclicking;
static bool _rclicking;
static bool _itemopen;

static int _tab;
static bool _tab_open;
static int _tab_x;
static bool state;


void framework::setup() {
	int w, h;
	engine_client()->get_screen_size(w, h);
	_window_size = { (float)w, (float)h };
	_size = { w / 2.f , h / 1.9f };
	_bounds = _size;
	_position = { (w / 2) - (_size.x / 2), (h / 2) - (_size.y / 2) };
	_dragging_position = _position;
	_alpha = 255.f;
	_color_background = { 55, 59, 61 };
	_color_partial = { 50,50,50 };
	_color_group = { 48,48,48 };
	_color_line = { 45,45,45 };
	_color_text = { 255,255,255 }; // 180,180,180

	_color_element_back = { 60,60,60 };
	_color_element_outline = { 70,70,70 };
}

const char* _groupbox_title = "";
int _groupbox_x = 0;
int _groupbox_y = 0;
int _mheight = 409;
int _aheight = 409;
int _groupbox_id = 0;
int _element = 0;
c_vector2d _offset;
int _previous = control_invalid;

int column = 0;
bool framework::run() {
	static bool init = false;

	if (!init) {
		setup();
		init = true;
		return false;
	}
	handle_keys();
	handle_movement();


	box(_position.x, _position.y, _size.x, _size.y, _color_background);
	box(_position.x, _position.y, _size.x, 20, _color_group);
	text(_position.x + 50, _position.y + 3, "bobhack alpha $", _color_text, true, 255, fnv1a("menu_font"));
	line(_position.x, _position.y + 20, _position.x + _size.x - 1, _position.y + 20, config.misc.theme);
	line(_position.x, _position.y + 21, _position.x + _size.x - 1, _position.y + 21, _color_line);
	outline(_position.x, _position.y, _size.x, _size.y, _color_line);

	_groupbox_x = 0;
	_groupbox_y = 0;
	_groupbox_id = 0;
	_mheight = _aheight = 409;
	_element = 1;
	column = 0;
	_previous = control_invalid;
}
int _sel_element = 0;
int framework::tabs(std::vector<std::string> tabs) {
	static bool switcingstate = true;


	if (_tab_open) { if (_tab_x >= 103) _tab_x = 105; _tab_x = lerp(0.1f, _tab_x, 105); }
	if (!_tab_open) { _tab_x = lerp(0.2f, _tab_x, 15); }

	bool should_switch = false;
	static bool pressed = false;
	bool h2 = (_inparam(_position.x, _position.y + 25, _tab_x, +(tabs.size() * 16)));
	bool hovered = (!h2 && _inparam(_position.x, _position.y + 23, _tab_x, _size.y - 23));


	if (!_clicking && hovered) {
		if (pressed)
			should_switch = true;
		pressed = false;
	}
	if (_clicking && hovered && !pressed && _sel_element == 0 && !_itemopen) pressed = true;




	static int __alpha = 200;
	__alpha = lerp(0.3f, __alpha, _tab_open ? 200 : 0);

	box(_position.x, _position.y + 22, _tab_x, _size.y - 22, _color_group);
	line(_position.x + _tab_x, _position.y + 21, _position.x + _tab_x, _position.y + _size.y, _color_line);




	int i = 0;
	for (const auto &it : tabs) {
		text(_position.x + (_tab_x / 2) - (_tab_open ? 0 : 1), _position.y + 25 + (i * 16), _tab_open ? it.c_str() : std::to_string(i + 1).c_str(), i == _tab ? config.misc.theme : _color_text, true, 255);
		if (_clicking && _inparam(_position.x, _position.y + 25 + (i * 16), _tab_x, 16) && !_itemopen && _sel_element == 0) {
			_tab = i;
		}
		i++;
	}
	if (should_switch)
		_tab_open = !_tab_open;
	return _tab;
}

bool hidden[16][16];
float hidden_alpha[16][16];
float group_slider[16][16];
float group_slider_lerp[16][16];
int nt_size;


int sel_groupbox = -1;
bool framework::groupbox(const char* title, int size, float scrollamount) {
	int s = size;
	if (size <= 0) {
		s = _aheight;
	}
	nt_size = s;
	groupbox_info info = { _position.x + 5 + _tab_x + _groupbox_x, _position.y + 26 + _groupbox_y, (_size.x - 19 - _tab_x) / 3, s, title };
	_groupbox_title = title;

	box(info.x, info.y, info.w, info.h, _color_partial);

	bool hovered = _inparam(info.x, info.y, info.w, 16) && !_itemopen && _sel_element == 0;
	bool hovered2 = _inparam(info.x, info.y, info.w, info.h) && !_itemopen && !hidden[_tab][_groupbox_id];

	static bool pressed[16][16];
	if (!_clicking && hovered) {
		if (pressed[_tab][_groupbox_id] && sel_groupbox == _groupbox_id)
			hidden[_tab][_groupbox_id] = !hidden[_tab][_groupbox_id];
		pressed[_tab][_groupbox_id] = false;
		sel_groupbox = -1;
	}
	if (_clicking && hovered && !pressed[_tab][_groupbox_id] && sel_groupbox == -1) {
		sel_groupbox = _groupbox_id; pressed[_tab][_groupbox_id] = true;
	}
	if (_clicking && !hovered) pressed[_tab][_groupbox_id] = false;
	if (!_clicking && !hovered && pressed[_tab][_groupbox_id]) { sel_groupbox = -1; };
	if (!_clicking && !hovered) { pressed[_tab][_groupbox_id] = false; };




	/* SCROLL BAR */
	if (scrollamount > 0) {
		float scroll_x = 4;
		float scroll_amount = scrollamount;

		if (group_slider[_tab][_groupbox_id] != group_slider_lerp[_tab][_groupbox_id])
			group_slider_lerp[_tab][_groupbox_id] = lerp(0.2f, group_slider_lerp[_tab][_groupbox_id], group_slider[_tab][_groupbox_id]);

		float _pxe = scroll_amount / (info.h - 18);
		box(info.x + info.w - scroll_x - 1, info.y + 17, scroll_x, info.h - 18,  _color_element_back);
		box(info.x + info.w - scroll_x - 1, info.y + 17, scroll_x, (group_slider_lerp[_tab][_groupbox_id] / _pxe), config.misc.theme, 100);

		if (_clicking && _inparam(info.x + info.w - scroll_x - 1, info.y + 17, scroll_x, info.h - 18)) _sel_element = (2000+ _groupbox_id) /* LOL */;
		if (!_clicking && _sel_element == (2000 + _groupbox_id)) _sel_element = 0;

		if (_sel_element == (2000 + _groupbox_id) && !_itemopen && _clicking) {
			float v = std::clamp(abs(_mouseposition().y - (info.y + 17)) * _pxe, 0.f, scroll_amount);
			if ((_mouseposition().y - (info.y + 17)) <= 0) v = 0;
			group_slider[_tab][_groupbox_id] = v;
		}

		if (group_slider[_tab][_groupbox_id] > scroll_amount) group_slider[_tab][_groupbox_id] = scroll_amount;
		if (group_slider[_tab][_groupbox_id] < 0) group_slider[_tab][_groupbox_id] = 0;


		if (GetAsyncKeyState(VK_DOWN) && hovered2) group_slider[_tab][_groupbox_id] += 2;
		if (GetAsyncKeyState(VK_UP) && hovered2) group_slider[_tab][_groupbox_id] -= 2;
		group_slider[_tab][_groupbox_id] = std::clamp(group_slider[_tab][_groupbox_id], 0.f, scroll_amount);

	}





	_offset = { info.x + 6, info.y + 22 };
	_offset.y -= group_slider[_tab][_groupbox_id];
	_previous = control_invalid;
	return true;
}


groupbox_info framework::get_info() {
	groupbox_info info = { _position.x + 5 + _tab_x + _groupbox_x, _position.y + 26 + _groupbox_y, (_size.x - 19 - _tab_x) / 3, nt_size };
	return info;
}

void framework::end_groupbox() {
	groupbox_info info = { _position.x + 5 + _tab_x + _groupbox_x, _position.y + 25 + _groupbox_y, (_size.x - 19 - _tab_x) / 3, nt_size };
	_aheight -= nt_size + 4;

	box(info.x, info.y, info.w, 16, _color_group);
	text(info.x + info.w / 2, info.y + 2, _groupbox_title, _color_text, true);
	line(info.x, info.y + 16, info.x + info.w - 2, info.y + 16, _color_line);
	box(info.x, info.y + info.h, info.w, 3, _color_background);
	box(info.x, info.y - 3, info.w, 3, _color_background);

	if (hidden_alpha[_tab][_groupbox_id] > 1 && false) {
		box(info.x, info.y + 17, info.w, info.h - 17, _color_partial, hidden_alpha[_tab][_groupbox_id]);
		text(info.x + info.w / 2, info.y + (info.h / 2) - 5, "< hidden >", _color_text, true, hidden_alpha[_tab][_groupbox_id]);
	}

	hidden_alpha[_tab][_groupbox_id] = lerp(0.1f, hidden_alpha[_tab][_groupbox_id], hidden[_tab][_groupbox_id] ? 260 : 0);

	outline(info.x, info.y, info.w, info.h, _color_line);

	_groupbox_y += nt_size + 4;
	_groupbox_id++;
}

float elements[516][516];

bool framework::checkbox(const char* title, bool *item) {

	auto info = get_info();
	_previous = control_checkbox;
	if (_offset.y + 10 < info.y + 16) { _element++; _offset.y += 14; return *item; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 14; return *item; }

	box(_offset.x, _offset.y, 10, 10, _color_element_back);
	box(_offset.x, _offset.y, 10, 10, config.misc.theme, elements[_tab][_element]);
	outline(_offset.x, _offset.y, 10, 10, _color_element_outline);

	elements[_tab][_element] = lerp(0.1f, elements[_tab][_element], *item ? 255 : 0);
	text(_offset.x + 15, _offset.y - 1, title, _color_text);
	
	auto text_size = renderer->get_text_size(title, fnv1a("menu_controls"));
	bool hovered = _inparam(_offset.x, _offset.y, 15 + text_size.x + 2, 10) && !_itemopen;
	static bool pressed[128];
	if (!_clicking && hovered) {
		if (pressed[_element] && _sel_element == _element)
			*item = !*item;
		pressed[_element] = false;
		_sel_element = 0;
	}
	if (_clicking && hovered && !pressed[_element] && !_itemopen && _sel_element == 0) {
		_sel_element = _element; pressed[_element] = true;
	}
	if (_clicking && !hovered) pressed[_element] = false;
	if (!_clicking && !hovered && pressed[_element]) { _sel_element = 0; };
	if (!_clicking && !hovered) { pressed[_element] = false; };

	_offset.y += 14;
	_element++;

	return *item;
}

float framework::slider(const char* title, float *item, float min, float max, const char* suffix, bool hidebar) {
	auto info = get_info();
	_offset.y -= 2;
	_previous = control_slider;
	if (_offset.y + 23 < info.y + 16) { _element++; _offset.y += 25; return *item; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 25; return *item; }


	float _pxe = max / (info.w - 32);

	text(_offset.x + 15, _offset.y, title, _color_text);
	box(_offset.x + 10, _offset.y + 14, info.w - 32, 6, _color_element_back);
	if (elements[_tab][_element] != *item)
		elements[_tab][_element] = lerp(0.1f, elements[_tab][_element], *item);



	std::string preview = std::to_string((int)std::roundf(*item)) + suffix;


	box(_offset.x + 10, _offset.y + 14, (elements[_tab][_element] / _pxe), 6, config.misc.theme);

	if (_lclicking && _inparam(_offset.x + 10, _offset.y + 14, info.w - 32, 6) && _sel_element == 0) _sel_element = _element;
	if (!_lclicking && _sel_element == _element) _sel_element = 0;

	if (_sel_element == _element && !_itemopen) {
		float v = std::clamp(abs(_mouseposition().x - (_offset.x + 10)) * _pxe, min, max);
		if ((_mouseposition().x - (_offset.x + 10)) <= min) v = min;
		*item = v;
	}
	*item = std::clamp(*item, min, max);


	outline(_offset.x + 10, _offset.y + 14, info.w - 32, 6, _color_element_outline);
	text(_offset.x + 10 + (elements[_tab][_element] / _pxe), _offset.y + 13, preview.c_str(), _color_text, true, _color_text.alpha);
	_element++;
	_offset.y += 25;

	return *item;

}
int framework::slider(const char* title, int *item, int min, int max, const char* suffix, bool hidebar) {
	auto info = get_info();
	_offset.y -= 2;
	_previous = control_slider;
	if (_offset.y + 23 < info.y + 16) { _element++; _offset.y += 25; return *item; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 25; return *item; }


	float _pxe = max / (info.w - 32);

	text(_offset.x + 15, _offset.y, title, _color_text);
	box(_offset.x + 10, _offset.y + 14, info.w - 32, 6, _color_element_back);
	if (elements[_tab][_element] != *item)
		elements[_tab][_element] = lerp(0.1f, elements[_tab][_element], *item);



	std::string preview = std::to_string((int)std::roundf(*item)) + suffix;


	box(_offset.x + 10, _offset.y + 14, (elements[_tab][_element] / _pxe), 6, config.misc.theme);

	if (_lclicking && _inparam(_offset.x + 10, _offset.y + 14, info.w - 32, 6) && _sel_element == 0) _sel_element = _element;
	if (!_lclicking && _sel_element == _element) _sel_element = 0;

	if (_sel_element == _element && !_itemopen) {
		float v = std::clamp(abs(_mouseposition().x - (_offset.x + 10)) * _pxe, (float)min, (float)max);
		if ((_mouseposition().x - (_offset.x + 10)) <= min) v = min;
		*item = v;
	}
	*item = std::clamp(*item, min, max);


	outline(_offset.x + 10, _offset.y + 14, info.w - 32, 6, _color_element_outline);
	text(_offset.x + 10 + (elements[_tab][_element] / _pxe), _offset.y + 13, preview.c_str(), _color_text, true, _color_text.alpha);
	_element++;
	_offset.y += 25;

	return *item;

}

int _element_type = 0;
std::vector<std::string> _element_items;
c_vector2d _element_offset;
int _element_combo = 0;
int _element_size = 0;
c_color _element_color = c_color(255, 255, 255, 255);
bool _element_multi[64];
int framework::combo(const char* title, int * item, std::vector<std::string> items) {
	auto info = get_info();
	_offset.y -= 2;
	_previous = control_combobox;
	if (_offset.y + 30 < info.y + 16) { _element++; _offset.y += 32; return *item; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 32; return *item; }

	bool hovered = _inparam(_offset.x + 10, _offset.y + 14, info.w - 32, 13) && (_sel_element == 0 || _sel_element == _element);
	static bool pressed[128];
	if (!_clicking && hovered) {
		if (pressed[_element]) {

			_itemopen = !_itemopen;
			_sel_element = _itemopen ? _element : 0;
			_element_type = _itemopen ? 1 : 0;
			if (_itemopen)
				_element_combo = *item;
		}
		pressed[_element] = false;

		if (!_itemopen && _sel_element == _element) {
			_sel_element = 0;
		}
	}
	if (_clicking && hovered && !pressed[_element]) {

		pressed[_element] = true;
	}
	if (!_clicking && !hovered) pressed[_element] = false;

	if (_sel_element == _element) {
		_element_items = items;

		_element_offset = _offset + c_vector2d(9, 26);

		*item = _element_combo;
	}

	text(_offset.x + 15, _offset.y, title, _color_text);
	box(_offset.x + 10, _offset.y + 14, info.w - 32, 13, _color_element_back);
	text(_offset.x + 15, _offset.y + 14, items[*item].c_str(), _color_text, false, 175);
	float ws = info.w - 32;
	_offset.y += 14;
	bool open = _sel_element == _element && _itemopen;
	line(_offset.x + ws - 5, _offset.y + 5, _offset.x + ws, _offset.y + 5, _color_text, 175);
	line(_offset.x + ws - (open ? 2 : 5), _offset.y + 7, _offset.x + ws, _offset.y + 7, _color_text, 175);
	_offset.y -= 14;
	outline(_offset.x + 10, _offset.y + 14, info.w - 32, 13, _color_element_outline);

	_element++;
	_offset.y += 32;

	return *item;
}
void framework::run_combo() {
	if (!_itemopen) return;
	if (_element_type != 1) return;
	auto info = get_info();
	float ws = info.w - 32;

	bool hovered = (_inparam(_element_offset.x + 1, _element_offset.y, ws, 12 * _element_items.size() + 2) || _inparam(_element_offset.x + 1, _element_offset.y - 26, ws, 27 + (12 * _element_items.size() + 2)));
	if (_clicking && !hovered) {
		_itemopen = !_itemopen;
		_sel_element = 0;
		_element_type = 0;
	}

	box(_element_offset.x + 1, _element_offset.y, ws, 12 * _element_items.size() + 2, _color_element_back);
	outline(_element_offset.x + 1, _element_offset.y, ws, 12 * _element_items.size() + 2, _color_element_outline);
	int i = 0;
	for (const auto &it : _element_items) {
		bool _hovered = _inparam(_element_offset.x + 1, _element_offset.y + (12 * i) + 1, ws, 12);
		text(_element_offset.x + 6, _element_offset.y + (12 * i) + 1, _element_items[i].c_str(), i == _element_combo ? config.misc.theme : _color_text, false, _hovered ? 255 : 175);
		if (_hovered && _clicking)
			_element_combo = i;

		i++;
	}
}

void framework::multicombo(const char* title, bool item[], std::vector<std::string> items) {
	auto info = get_info();
	_offset.y -= 2;
	if (_previous == control_invalid) _offset.y -= 2;
	_previous = control_multicombo;
	if (_offset.y + 30 < info.y + 16) { _element++; _offset.y += 32; return; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 32; return; }

	bool hovered = _inparam(_offset.x + 10, _offset.y + 14, info.w - 32, 13) && (_sel_element == 0 || _sel_element == _element);
	static bool pressed[128];
	if (!_clicking && hovered) {
		if (pressed[_element]) {

			_itemopen = !_itemopen;
			_sel_element = _itemopen ? _element : 0;
			_element_type = _itemopen ? 2 : 0;
		}
		pressed[_element] = false;

		if (!_itemopen && _sel_element == _element) {
			_sel_element = 0;
		}
	}
	if (_clicking && hovered && !pressed[_element]) {

		pressed[_element] = true;
	}
	if (!_clicking && !hovered) pressed[_element] = false;

	if (_sel_element == _element) {
		_element_items = items;

		_element_offset = _offset + c_vector2d(9, 26);
		{
			int i = 0;
			for (const auto &it : items) {
				item[i] = _element_multi[i];
				i++;
			}
		}


	}

	text(_offset.x + 15, _offset.y, title, _color_text);
	box(_offset.x + 10, _offset.y + 14, info.w - 32, 13, _color_element_back);
	int _sel_amount = 0;
	std::string preview = "";
	int i = 0;
	for (const auto &it : items) {
		if (item[i] == true) {
			_sel_amount++;
			if (_sel_amount == 1) preview += it;
			else preview += ", " + it;
		}
		i++;
	}

	text(_offset.x + 15, _offset.y + 14, preview.c_str(), _color_text, false, 175);


	float ws = info.w - 32;
	_offset.y += 14;
	bool open = _sel_element == _element && _itemopen;
	line(_offset.x + ws - 5, _offset.y + 5, _offset.x + ws, _offset.y + 5, _color_text, 175);
	line(_offset.x + ws - (open ? 2 : 5), _offset.y + 7, _offset.x + ws, _offset.y + 7, _color_text, 175);
	_offset.y -= 14;
	outline(_offset.x + 10, _offset.y + 14, info.w - 32, 13, _color_element_outline);

	_element++;
	_offset.y += 32;

	return;
}

void framework::run_multicombo() {
	if (!_itemopen) return;
	if (_element_type != 2) return;
	auto info = get_info();
	float ws = info.w - 32;

	bool hovered = (_inparam(_element_offset.x + 1, _element_offset.y, ws, 12 * _element_items.size() + 2) || _inparam(_element_offset.x + 1, _element_offset.y - 26, ws, 27 + (12 * _element_items.size() + 2)));
	if (_clicking && !hovered) {
		_itemopen = !_itemopen;
		_sel_element = 0;
		_element_type = 0;
	}

	box(_element_offset.x + 1, _element_offset.y, ws, 12 * _element_items.size() + 2, _color_element_back);
	outline(_element_offset.x + 1, _element_offset.y, ws, 12 * _element_items.size() + 2, _color_element_outline);
	int i = 0;
	static bool pressed[128];
	for (const auto &it : _element_items) {
		bool _hovered = _inparam(_element_offset.x + 1, _element_offset.y + (12 * i) + 1, ws, 12);
		text(_element_offset.x + 6, _element_offset.y + (12 * i) + 1, _element_items[i].c_str(), _element_multi[i] ? config.misc.theme : _color_text, false, _hovered ? 255 : 175);



		if (!_clicking && _hovered) {
			if (pressed[i]) _element_multi[i] = !_element_multi[i];
			pressed[i] = false;
		}
		if (_clicking && _hovered && !pressed[i]) pressed[i] = true;
		if (_clicking && !_hovered) pressed[i] = false;


		i++;
	}
}

void framework::next_column() {
	column++;
	_groupbox_x = column * (((_size.x - 19 - _tab_x) / 3) + 5);
	_groupbox_y = 0;
	_aheight = _mheight;
}

int old_color = 0;
c_color _copied = c_color(255, 255, 255, 255);

c_color _copied_drag = c_color(255, 255, 255, 255);

bool _copied_dragging = false;
int _copied_id = 0;
bool _copied_pasted = false;

c_color framework::colors(c_color* item) {
	auto info = get_info();

	int old_offset = 10;
	int offsetx = 0;
	auto old_previous = _previous;
	switch (_previous) {
	case control_invalid: default: old_offset = 10; offsetx = 0; break;
	case control_checkbox: old_offset = 14; offsetx = 0; break;
	case control_slider: old_offset = 25; offsetx = 0; break;
	case control_combobox: case control_multicombo: old_offset = 32; offsetx = 0; break;
	case control_color: old_offset = old_color; offsetx = 18; break;
	};
	old_color = old_offset;
	_previous = control_color;
	if (_offset.y + 8 < info.y + 16) { _element++;  return *item; }
	if (_offset.y > info.y + info.h) { _element++; return *item; }

	_offset.y -= old_offset;
	float ws = info.w - 26;
	c_vector2d c_pos = { _offset.x + ws - offsetx, _offset.y + 1 };

	bool hovered = _inparam(c_pos.x, c_pos.y, 14, 8) && (_sel_element == 0 || _sel_element == _element);
	static bool pressed[128];
	static bool pressedr[128];

	if (_sel_element == _element && (_element_type == 3 || _element_type == 4)) {
		if (_element_color.red != item->red || _element_color.green != item->green || _element_color.blue != item->blue || _element_color.alpha != item->alpha)
			*item = _element_color;

		_element_offset = c_pos + c_vector2d(18, 0);
		_element_color = *item;

	}

	if (!_lclicking && hovered) {
		if (pressed[_element]) {

			_itemopen = !_itemopen;
			_sel_element = _itemopen ? _element : 0;
			_element_type = _itemopen ? 3 : 0;
			if (_itemopen)
				_element_color = *item;
		}
		pressed[_element] = false;

		if (!_itemopen && _sel_element == _element) {
			_sel_element = 0;
		}
	}
	if (_lclicking && hovered && !pressed[_element]) {

		pressed[_element] = true;
	}
	if (!_lclicking && !hovered) pressed[_element] = false;

	if (pressed[_element] && !hovered && _lclicking) {
		_copied_dragging = true;
		_copied_id = _element;
		_copied_drag = *item;
		_sel_element = _element;
		_element_type = 5;
	}




	if (!_rclicking && hovered) {
		if (pressedr[_element]) {
			_itemopen = !_itemopen;
			_sel_element = _itemopen ? _element : 0;
			_element_type = _itemopen ? 4 : 0;
			if (_itemopen)
				_element_color = *item;
		}
		pressedr[_element] = false;

		if (!_itemopen && _sel_element == _element) {
			_sel_element = 0;
		}
	}
	if (_rclicking && hovered && !pressedr[_element]) {

		pressedr[_element] = true;
	}
	if (!_rclicking && !hovered) pressedr[_element] = false;

	if (!_itemopen && _sel_element == _element && (_element_type == 3 || _element_type == 4)) {
		_sel_element = 0;
		_element_type = 0;
	}



	box(c_pos.x, c_pos.y, 14, 8, _color_element_back);
	box(c_pos.x, c_pos.y, 14, 8, *item, item->alpha);
	outline(c_pos.x, c_pos.y, 14, 8, _color_element_outline);


	if (_copied_dragging && _copied_id != _element && _element_type == 5 &&
		_inparam(c_pos.x - 2, c_pos.y - 2, 18, 12)) {
		auto _c_pos = _mouseposition();
		// new box
		box(c_pos.x, c_pos.y, 14, 8, _color_element_back);
		box(c_pos.x, c_pos.y, 14, 8, _copied_drag, _copied_drag.alpha);
		outline(c_pos.x, c_pos.y, 14, 8, _color_element_outline);
		if (!_lclicking) {
			*item = _copied_drag;
			_copied_dragging = false;
			_copied_id = 0;
			_sel_element = 0;
			_element_type = 0;
		}
	}



	if (_copied_dragging && _copied_id == _element && _element_type == 5) {
		if ((_escclicking || _rclicking) && !_lclicking) {
			_copied_dragging = false;
			_copied_id = 0;
			_sel_element = 0;
			_element_type = 0;
		}
	}


	_element++;
	_offset.y += old_offset;
	return *item;
}

void framework::run_color() {
	if (!_itemopen) return;
	if (_element_type != 3) return;
	auto info = get_info();
	auto cpos = _element_offset;
	static int _sel_bar = 0;
	bool hovered = (_inparam(cpos.x, cpos.y, 137, 117) || _inparam(cpos.x - 18, cpos.y, 14, 8));
	if (_clicking && !hovered && _sel_bar == 0) {
		_itemopen = !_itemopen;
		_sel_element = 0;
		_element_type = 0;
	}

	

	box(cpos.x, cpos.y, 137, 117, _color_group);
	outline(cpos.x, cpos.y, 137, 117, _color_element_outline);

	box(cpos.x + 3, cpos.y + 3, 99, 99, _color_element_back);
	c_color picker;
	int item = 0;
	int item_x = 4;
	int item_y = 4;
	for (int i = 0; i < 100; i++) {
		if (item == 10) { item = 0; item_y = 4; item_x += 10; }
		float hue = (i * 0.01f);
		if (i < 98) picker.fuckloki(hue, 1.f, 1.f);
		if (i == 98) picker = c_color(255, 255, 255, 255);
		if (i == 99) picker = c_color(0, 0, 0, 255);
		picker.alpha = 255;
		box(cpos.x + item_x, cpos.y + item_y, 8, 8, picker);
		if (_clicking && _inparam(cpos.x + item_x, cpos.y + item_y, 8, 8) && _sel_bar == 0)
			_element_color = picker;
		item_y += 10;
		item++;
	}
	outline(cpos.x + 3, cpos.y + 3, 100, 100, _color_element_outline);
	float _pxe = 255 / 99;
	static float red_lerp = 0.f;
	static float green_lerp = 0.f;
	static float blue_lerp = 0.f;
	static float alpha_lerp = 0.f;

	red_lerp = lerp(0.2f, red_lerp, _element_color.red);
	green_lerp = lerp(0.2f, green_lerp, _element_color.green);
	blue_lerp = lerp(0.2f, blue_lerp, _element_color.blue);
	alpha_lerp = lerp(0.2f, alpha_lerp, _element_color.alpha);
	// RED
	{
		box(cpos.x + 7 + 99, cpos.y + 3, 6, 99, _color_element_back);
		box(cpos.x + 7 + 99, cpos.y + 3, 6, std::clamp((red_lerp / _pxe), 0.f, 99.f), c_color(_element_color.red, 0, 0));
		outline(cpos.x + 7 + 99, cpos.y + 3, 6, 99, _color_element_outline);
		bool hovered = (_inparam(cpos.x + 7 + 99, cpos.y + 3, 6, 99));

		if (_lclicking && hovered) _sel_bar = 1;
		if (!_lclicking && _sel_bar == 1) _sel_bar = 0;

		if (_sel_bar == 1) {
			float v = std::clamp(abs(_mouseposition().y - (cpos.y + 3)) * _pxe, 0.f, 255.f);
			if ((_mouseposition().y - (cpos.y + 3)) <= 0) v = 0;
			_element_color.red = v;
		}
		_element_color.red = std::clamp(_element_color.red, 0, 255);
	}
	// GREEN
	{
		box(cpos.x + 17 + 99, cpos.y + 3, 6, 99, _color_element_back);
		box(cpos.x + 17 + 99, cpos.y + 3, 6, std::clamp((green_lerp / _pxe), 0.f, 99.f), c_color(0, _element_color.green, 0));
		outline(cpos.x + 17 + 99, cpos.y + 3, 6, 99, _color_element_outline);
		bool hovered = (_inparam(cpos.x + 17 + 99, cpos.y + 3, 6, 99));

		if (_lclicking && hovered) _sel_bar = 2;
		if (!_lclicking && _sel_bar == 2) _sel_bar = 0;

		if (_sel_bar == 2) {
			float v = std::clamp(abs(_mouseposition().y - (cpos.y + 3)) * _pxe, 0.f, 255.f);
			if ((_mouseposition().y - (cpos.y + 3)) <= 0) v = 0;
			_element_color.green = v;
		}
		_element_color.green = std::clamp(_element_color.green, 0, 255);
	}
	// BLUE
	{
		box(cpos.x + 27 + 99, cpos.y + 3, 6, 99, _color_element_back);
		box(cpos.x + 27 + 99, cpos.y + 3, 6, std::clamp((blue_lerp / _pxe), 0.f, 99.f), c_color(0, 0, _element_color.blue));
		outline(cpos.x + 27 + 99, cpos.y + 3, 6, 99, _color_element_outline);
		bool hovered = (_inparam(cpos.x + 27 + 99, cpos.y + 3, 6, 99));

		if (_lclicking && hovered) _sel_bar = 3;
		if (!_lclicking && _sel_bar == 3) _sel_bar = 0;

		if (_sel_bar == 3) {
			float v = std::clamp(abs(_mouseposition().y - (cpos.y + 3)) * _pxe, 0.f, 255.f);
			if ((_mouseposition().y - (cpos.y + 3)) <= 0) v = 0;
			_element_color.blue = v;
		}
		_element_color.blue = std::clamp(_element_color.blue, 0, 255);
	}
	// ALPHA
	{
		box(cpos.x + 3, cpos.y + 7 + 99, 99, 6, _color_element_back);
		box(cpos.x + 3, cpos.y + 7 + 99, std::clamp((alpha_lerp / _pxe), 0.f, 99.f), 6, c_color(_element_color.alpha, _element_color.alpha, _element_color.alpha, 255));
		outline(cpos.x + 3, cpos.y + 7 + 99, 99, 6, _color_element_outline);
		bool hovered = (_inparam(cpos.x + 3, cpos.y + 7 + 99, 99, 6));

		if (_lclicking && hovered) _sel_bar = 4;
		if (!_lclicking && _sel_bar == 4) _sel_bar = 0;

		if (_sel_bar == 4) {
			float v = std::clamp(abs(_mouseposition().x - (cpos.x + 3)) * _pxe, 0.f, 255.f);
			if ((_mouseposition().x - (cpos.x + 3)) <= 0) v = 0;
			_element_color.alpha = (int)v;
		}
		_element_color.alpha = std::clamp(_element_color.alpha, 0, 255);
	}

	box(cpos.x + 7 + 99, cpos.y + 7 + 99, 26, 6, _element_color);
	outline(cpos.x + 7 + 99, cpos.y + 7 + 99, 26, 6, _color_element_outline);
}

void framework::run_color_dropdown() {
	if (!_itemopen) return;
	if (_element_type != 4) return;
	auto info = get_info();
	auto cpos = _element_offset;



	box(cpos.x, cpos.y, 40, 26, _color_group);
	outline(cpos.x, cpos.y, 40, 28, _color_element_outline);
	bool _hovered = (_inparam(cpos.x, cpos.y, 40, 28) || _inparam(cpos.x - 18, cpos.y, 14, 8));
	if (_clicking && !_hovered) {
		_itemopen = !_itemopen;
		_sel_element = 0;
		_element_type = 0;
	}
	bool hovered = _inparam(cpos.x, cpos.y, 40, 13);
	bool hovered2 = _inparam(cpos.x, cpos.y + 13, 40, 13);
	text(cpos.x + 20, cpos.y + 1, "copy", hovered ? config.misc.theme : c_color(255, 255, 255), true);
	text(cpos.x + 20, cpos.y + 13, "paste", hovered2 ? config.misc.theme : c_color(255, 255, 255), true);
	bool close = false;
	if (hovered && _clicking) { _copied = _element_color; close = true; }
	if (hovered2 && _clicking) { _element_color = _copied; close = true; }

	if (close) {
		_itemopen = !_itemopen;
	};
}

void framework::run_color_drag() {
	if (!_copied_dragging) return;
	if (_element_type != 5) return;
	auto _c_pos = _mouseposition();
	box(_c_pos.x, _c_pos.y, 14, 8, _color_element_back, 100);
	box(_c_pos.x, _c_pos.y, 14, 8, _copied_drag, 100);
	outline(_c_pos.x, _c_pos.y, 14, 8, _color_element_outline, 100);

	if (!_lclicking) {
		_copied_dragging = false;
		_copied_id = 0;
		_sel_element = 0;
		_element_type = 0;
	}
}


const char* const KeyNames[] = {
	"?",
	"m1",
	"m2",
	"cancel",
	"m3",
	"m4",
	"m5",
	"?",
	"back",
	"tab",
	"?",
	"?",
	"clear",
	"return",
	"?",
	"?",
	"shift",
	"control",
	"menu",
	"pause",
	"capital",
	"kana",
	"?",
	"junja",
	"final",
	"kanji",
	"?",
	"escape",
	"convert",
	"non-convert",
	"accept",
	"modechange",
	"space",
	"prior",
	"next",
	"end",
	"home",
	"left",
	"up",
	"right",
	"down",
	"select",
	"print",
	"execute",
	"snapshot",
	"insert",
	"delete",
	"help",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",
	"win",
	"win",
	"apps",
	"?",
	"sleep",
	"numpad 0",
	"numpad 1",
	"numpad 2",
	"numpad 3",
	"numpad 4",
	"numpad 5",
	"numpad 6",
	"numpad 7",
	"numpad 8",
	"numpad 0",
	"multiply",
	"add",
	"seperator",
	"suntract",
	"decimal",
	"divide",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"f10",
	"f11",
	"f12",
	"f13",
	"f14",
	"f15",
	"f16",
	"f17",
	"f18",
	"f19",
	"f20",
	"f21",
	"f22",
	"f23",
	"f24",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"numlock",
	"scrolllock",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"shift",
	"shift",
	"control",
	"control",
	"l-menu",
	"r-menu"
};

void framework::hotkey(const char* title, int * item) {
	auto info = get_info();
	_offset.y -= 2;
	if (_previous == control_invalid) _offset.y -= 2;
	_previous = control_checkbox;
	if (_offset.y + 30 < info.y + 16) { _element++; _offset.y += 16; return; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 16; return; }

	bool hovered = _inparam(_offset.x + 10, _offset.y, info.w - 32, 13) && (_sel_element == 0 || _sel_element == _element);
	static bool pressed[128];
	if (!_clicking && hovered) {
		if (pressed[_element]) {

			_itemopen = !_itemopen;
			_sel_element = _itemopen ? _element : 0;
			_element_type = _itemopen ? 7 : 0;
		}
		pressed[_element] = false;

		if (!_itemopen && _sel_element == _element) {
			_sel_element = 0;
		}
	}
	if (_clicking && hovered && !pressed[_element]) {

		pressed[_element] = true;
	}
	if (!_clicking && !hovered) pressed[_element] = false;

	if (_sel_element == _element) {
		

		_element_offset = _offset + c_vector2d(9, 26);
		*item = _element_combo;
		


	}

	text(_offset.x + 15, _offset.y, title, _color_text);
	
	std::string preview = "";
	
	if (*item == 0) preview = "< - >"; else { preview += "< "; preview += KeyNames[*item]; preview += " >"; }
	if (_sel_element == _element) preview = "< press a key >";
	

	text(_offset.x + 15 + (info.w-32) - renderer->get_text_size(preview.c_str(), fnv1a("menu_controls")).x, _offset.y, preview.c_str(), _sel_element == _element ? config.misc.theme : _color_text, false, 175);

	if (_sel_element == _element && _element_type == 7) {
		if (GetAsyncKeyState(VK_LBUTTON) && _itemopen) _itemopen = !_itemopen;
		if (GetAsyncKeyState(VK_ESCAPE) && _itemopen) {_itemopen = !_itemopen;}
		if (GetAsyncKeyState(VK_RBUTTON) && _itemopen) {*item = VK_RBUTTON; _itemopen = !_itemopen;}
		if (GetAsyncKeyState(VK_MBUTTON) && _itemopen) {*item = VK_MBUTTON; _itemopen = !_itemopen;}
		if (GetAsyncKeyState(VK_XBUTTON1) && _itemopen) {*item = VK_XBUTTON1; _itemopen = !_itemopen;}
		if (GetAsyncKeyState(VK_XBUTTON2) && _itemopen) {*item = VK_XBUTTON2; _itemopen = !_itemopen;}


		for (int key = VK_BACK; key < VK_RMENU; key++) {
			if (GetAsyncKeyState(key) && _itemopen) {
				*item = key; _itemopen = !_itemopen;
			}
		}
	}



	_element++;
	_offset.y += 16;

	return;
}

void framework::text(const char* title) {
	auto info = get_info();
	_offset.y -= 2;
	if (_previous == control_invalid) _offset.y -= 2;
	_previous = control_checkbox;
	if (_offset.y + 30 < info.y + 16) { _element++; _offset.y += 16; return; }
	if (_offset.y > info.y + info.h) { _element++; _offset.y += 16; return; }

	

	text(_offset.x + 15, _offset.y, title, _color_text);




	_element++;
	_offset.y += 16;

	return;
}
// rendering
void framework::box(int x, int y, int w, int h, c_color c, int max) {
	//render::get().draw_filled_rect(x, y, w, h, handle_color(c, max));
	renderer->rect_filled(c_vector2d(x, y), c_vector2d(w, h), handle_color(c, max));
}

void framework::outline(int x, int y, int w, int h, c_color c, int max) {
	//render::get().draw_outline(x, y, w, h, handle_color(c, max));
	renderer->rect(c_vector2d(x, y), c_vector2d(w, h), handle_color(c, max));
}

void framework::text(int x, int y, const char* string, c_color c, bool center, int max, unsigned long font) {
	//render::get().draw_text(x, y, font, string, center, handle_color(c, max));

	if(center)
		renderer->text(c_vector2d(x, y), string, handle_color(c, max), font, c_font::font_flags::centered_x);
	else
		renderer->text(c_vector2d(x, y), string, handle_color(c, max), font);
}

c_vector2d framework::text_size(const char* text, unsigned long font) {
	return { 0, 0 };
}

void framework::line(int x, int y, int x2, int y2, c_color c, int max) {
	//render::get().draw_line(x, y, x2, y2, handle_color(c, max));
	renderer->line(c_vector2d(x, y), c_vector2d(x2, y2), handle_color(c, max));
}

// main functions
void framework::handle_movement() {
	bool _ispressed = false;
	if (_clicking && !_itemopen && _sel_element == 0) _ispressed = true;
	if (_dragging && !_ispressed) _dragging = false;


	//_alpha = lerp(0.2f, _alpha, _dragging ? 200 : 255);

	c_vector2d _mousepos = _mouseposition();

	if (_dragging && _ispressed) {
		_position.x = _mousepos.x - _dragging_position.x;
		_position.y = _mousepos.y - _dragging_position.y;
	}
	if (_inparam(_position.x, _position.y, _size.x, 20)) {
		_dragging = true;
		_dragging_position.x = _mousepos.x - _position.x;
		_dragging_position.y = _mousepos.y - _position.y;
	}

	if (_position.x < 0) _position.x = 0;
	if (_position.y < 0) _position.y = 0;
	if (_position.x + _bounds.x > _window_size.x) _position.x = _window_size.x - _bounds.x;
	if (_position.y + _bounds.y > _window_size.y) _position.y = _window_size.y - _bounds.y;

}

c_color framework::handle_color(c_color c, int max) {

	return c_color(c.red, c.green, c.blue, std::clamp(_alpha, 0.f, (float)max));
}



void framework::handle_keys() {
	// MENU OPEN
	static bool _pressed = true;

	if (!_pressed && GetAsyncKeyState(VK_INSERT))
		_pressed = true;
	else if (_pressed && !GetAsyncKeyState(VK_INSERT))
	{
		_pressed = false;
		state = !state;


		//interfaces::inputsystem->enable_input(!state);
	}

	_lclicking = GetAsyncKeyState(VK_LBUTTON);
	_rclicking = GetAsyncKeyState(VK_RBUTTON);
	_ctrlclicking = GetAsyncKeyState(VK_CONTROL);
	_escclicking = GetAsyncKeyState(VK_ESCAPE);
	_clicking = _lclicking || _rclicking;
}

c_vector2d framework::_mouseposition()
{
	POINT mouse_position;
	GetCursorPos(&mouse_position);
	ScreenToClient(FindWindow(0, "Counter-Strike: Global Offensive"), &mouse_position);
	return { static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y) };
}
bool framework::_inparam(int x, int y, int x2, int y2) {
	if (_mouseposition().x > x && _mouseposition().y > y && _mouseposition().x < x2 + x && _mouseposition().y < y2 + y)
		return true;
	return false;
}

float framework::lerp(float t, float a, float b) {
	return (1 - t)*a + t * b;
}





/*
	// begin menu rendering
	if (!c_loki::menu_begin()) return;
	// define tab
	static int menu_tab = 0;

	// do tabs
	c_loki::menu_tabs(&menu_tab, { "ragebot", "antiaim", "esp", "models",  "miscellaneous", "players" });

	c_loki::setcolumns(3);

	// ragebot
	if (menu_tab == 0) {
		static int wep = 0;
		auto cfg = &config.rage.auto_snipe;
		switch (wep) {
		case 0: cfg = &config.rage.auto_snipe; break;
		case 1: cfg = &config.rage.scout; break;
		case 2: cfg = &config.rage.awp; break;
		case 3: cfg = &config.rage.pistol; break;
		case 4: cfg = &config.rage.misc; break;
		}

		int o = c_loki::menu_subtabs(&wep, { "auto", "scout", "awp", "pistol", "default" });
		c_loki::add_usage(o);



	}
	// antiaim
	else if (menu_tab == 1) {
		if (c_loki::menu_group(200, "antiaim")) {
			c_loki::checkbox("active", &config.rage.antiaim_settings.enabled, "enable antiaim");
			c_loki::hotkey("slow-walk", &config.rage.slow_walk, "key required to slow-walk");
			c_loki::hotkey("fake-duck", &config.rage.fake_duck, "key required to fake-duck");
			c_loki::hotkey("inverter", &config.misc.inverter, "key required to invert fake angle");
			c_loki::hotkey("switch manual", &config.misc.flip, "key required to switch manual antiaim");
		}
		if (c_loki::menu_group(-1, "fakelag")) {
			c_loki::checkbox("enabled", &config.rage.fakelag_settings.enabled, "enable fakelag");
			c_loki::checkbox("adaptive", &config.rage.fakelag_settings.fake_lag_automatic, "preset fakelag values");
			c_loki::slider("standing", &config.rage.fakelag_settings.fake_lag_standing, 1, 16, "t", false, "fakelag amount while standing");
			c_loki::slider("moving", &config.rage.fakelag_settings.fake_lag_moving, 1, 16, "t", false, "fakelag amount while moving");
			c_loki::slider("in air", &config.rage.fakelag_settings.fake_lag_air, 1, 16, "t", false, "fakelag amount while in-air");
			c_loki::slider("slow walking", &config.rage.fakelag_settings.fake_lag_slowwalk, 1, 16, "t", false, "fakelag amount while slow-walking");

			c_loki::checkbox("peek delay", &config.rage.fakelag_settings.fake_lag_on_peek_delay, "allow lagged position to sync with actual position before shooting");
			c_loki::checkbox("disable on shot", &config.rage.fakelag_settings.disable_on_shooting, "disable fakelag while shooting");
			c_loki::checkbox("disable on revolver", &config.rage.fakelag_settings.disable_on_revolver, "disable fakelag with revolver");
			c_loki::checkbox("disable on taser ", &config.rage.fakelag_settings.disable_on_tazer, "disable fakelag with taser");
			c_loki::checkbox("disable on grenade", &config.rage.fakelag_settings.disable_on_grenade, "disable fakelag with grenades");
			c_loki::checkbox("disable on knife", &config.rage.fakelag_settings.disable_on_knife, "disable fakelag with a knife");
		}
		c_loki::menu_next_column();

		if (c_loki::menu_group(200, "standing")) {
			if (c_loki::combo("standing pitch", &config.rage.antiaim_settings.pitch_mode, { "off","down", "zero", "up", "custom" }) == 4)
				c_loki::slider("custom pitch", &config.rage.antiaim_settings.custom_pitch, 0.f, 360.f, "");

			c_loki::combo("standing yaw", &config.rage.antiaim_settings.yaw_mode, { "off","static", "jitter", "freestanding", "manual" });

			c_loki::slider("additive", &config.rage.antiaim_settings.yaw_add, 0.f, 360.f, "");
			if (config.rage.antiaim_settings.yaw_mode == 2)
				c_loki::slider("jitter range", &config.rage.antiaim_settings.jitter_range, 0.f, 180.f, "");


		}
		if (c_loki::menu_group(-1, "moving")) {
			if (c_loki::combo("moving pitch", &config.rage.antiaim_settings.pitch_moving_mode, { "off","down", "zero", "up", "custom" }) == 4)
				c_loki::slider("custom pitch", &config.rage.antiaim_settings.custom_moving_pitch, 0.f, 360.f, "");

			c_loki::combo("moving yaw", &config.rage.antiaim_settings.yaw_moving_mode, { "off","static", "jitter", "freestanding", "manual" });
			c_loki::slider("additive", &config.rage.antiaim_settings.yaw_moving_add, 0.f, 360.f, "");

			if (config.rage.antiaim_settings.yaw_moving_mode == 2)
				c_loki::slider("jitter range", &config.rage.antiaim_settings.jitter_moving_range, 0.f, 180.f, "");


		}
		c_loki::menu_next_column();

		if (c_loki::menu_group(200, "slow walking")) {
			if (c_loki::combo("slow pitch", &config.rage.antiaim_settings.pitch_slow_mode, { "off","down", "zero", "up", "custom" }) == 4)
				c_loki::slider("custom pitch", &config.rage.antiaim_settings.custom_slow_pitch, 0.f, 360.f, "");
			c_loki::combo("slow yaw", &config.rage.antiaim_settings.yaw_slow_mode, { "off","static", "jitter", "freestanding", "manual" });
			c_loki::slider("additive", &config.rage.antiaim_settings.yaw_slow_add, 0.f, 360.f, "");

			if (config.rage.antiaim_settings.yaw_slow_mode == 2)
				c_loki::slider("jitter range", &config.rage.antiaim_settings.jitter_slow_range, 0.f, 180.f, "");


		}
		if (c_loki::menu_group(-1, "in air")) {
			if (c_loki::combo("in air pitch", &config.rage.antiaim_settings.pitch_air_mode, { "off","down", "zero", "up", "custom" }) == 4)
				c_loki::slider("custom pitch", &config.rage.antiaim_settings.custom_air_pitch, 0.f, 360.f, "");

			c_loki::combo("in air yaw", &config.rage.antiaim_settings.yaw_air_mode, { "off","static", "jitter", "freestanding", "manual" });
			c_loki::slider("additive", &config.rage.antiaim_settings.yaw_air_add, 0.f, 360.f, "");

			if (config.rage.antiaim_settings.yaw_air_mode == 2)
				c_loki::slider("jitter range", &config.rage.antiaim_settings.jitter_air_range, 0.f, 180.f, "");


		}

	}
	// esp
	else if (menu_tab == 2) {
	c_loki::setcolumns(2);
	static int esp_tab = 0;
	int o = c_loki::menu_subtabs(&esp_tab, { "player", "other" });
	c_loki::add_usage(o);

		if(esp_tab == 0){
		if (c_loki::menu_group(-1, "players")) {

			c_loki::checkbox("name tag", &config.esp.enemy.name);
			c_loki::coloralpha("color", &config.esp.name_color, false);

			c_loki::checkbox("bounding box", &config.esp.enemy.box);
			c_loki::coloralpha("color", &config.esp.box_color, false);

			c_loki::checkbox("health bar", &config.esp.enemy.health);
			c_loki::coloralpha("color", &config.esp.health_color, false);

			c_loki::checkbox("armor bar", &config.esp.armor);
			c_loki::coloralpha("color", &config.esp.armor_color, false);

			c_loki::checkbox("weapon", &config.esp.enemy.weapon);
			c_loki::coloralpha("color", &config.esp.weapon_color, false);

			c_loki::checkbox("ammo", &config.esp.enemy.ammo);
			c_loki::coloralpha("color", &config.esp.ammo_color, false);

			c_loki::checkbox("skeleton", &config.esp.enemy.skeleton);
			c_loki::coloralpha("color", &config.esp.skeleton_color, false);


		}
		c_loki::menu_next_column();
		c_loki::add_usage(o);
		if (c_loki::menu_group(98, "flags")) {
			c_loki::checkbox("scoped", &config.esp.enemy_flags.zoom);
			c_loki::checkbox("reload", &config.esp.enemy_flags.reload);
			c_loki::checkbox("defuse", &config.esp.enemy_flags.defuse);
			c_loki::checkbox("armor", &config.esp.enemy_flags.kevlar);
			c_loki::checkbox("shots", &config.esp.enemy_flags.resolver);
		}
		if (c_loki::menu_group(95, "style")) {

			c_loki::checkbox("health based bar", &config.esp.health_based);
			c_loki::slider("health dividers", &config.esp.health_dividers, 0, 10, "");
			c_loki::checkbox("weapon icons", &config.esp.wep_icons);
			c_loki::checkbox("backtrack skeleton", &config.esp.enemy.history_skeleton);
		}
		if (c_loki::menu_group(-1, "other players")) {

			c_loki::checkbox("enemy bullet tracers", &config.esp.enemy.impacts);
			c_loki::coloralpha("color", &config.esp.impacts_color, false);
			c_loki::checkbox("visualize lag", &config.esp.show_lag_compensation);
			c_loki::coloralpha("color", &config.esp.show_lag_compensation_color, false);
			c_loki::checkbox("visualize multipoint", &config.esp.multipoint);
			c_loki::checkbox("visualize on-shot", &config.esp.show_on_shot_hitboxes);

		}
		}
		else if (esp_tab == 1) {
			if (c_loki::menu_group(127, "removals")) {
				c_loki::checkbox("scope overlay", &config.misc.no_scope);
				c_loki::checkbox("smoke", &config.misc.no_smoke);
				c_loki::checkbox("fog", &config.misc.no_fog);
				c_loki::checkbox("flashbang effects", &config.misc.no_flash);
				c_loki::checkbox("zoom", &config.misc.remove_zoom);
				c_loki::checkbox("recoil", &config.misc.no_recoil);
				c_loki::checkbox("punch", &config.misc.remove_punch);
				c_loki::checkbox("post processing", &config.misc.no_post_processing);
			}
			if (c_loki::menu_group(-1, "other esp")) {
				c_loki::checkbox("local bullet tracers", &config.esp.local_impact);
				c_loki::coloralpha("color", &config.esp.local_impacts_color, false);
				c_loki::slider("thickness", &config.esp.local_impact_width, 0, 20, "");
				c_loki::checkbox("grenades", &config.esp.nade_esp);
				c_loki::coloralpha("color", &config.esp.nade_color, false);
				c_loki::checkbox("grenade path", &config.esp.grenade_pred);
				c_loki::coloralpha("color", &config.esp.grenade_predc, false);
				c_loki::combo("grenade path style", &config.esp.grenade_preds, { "simple", "advanced", "box" });
			}
			c_loki::menu_next_column();
			c_loki::add_usage(o);
			if (c_loki::menu_group(75, "effects")) {
				c_loki::checkbox("nightmode", &config.misc.nightmode);
				c_loki::slider("darkness", &config.misc.nightmode_darkness, 1, 100, "", true);
				c_loki::checkbox("full bright", &config.misc.full_bright);
			}

		}



	}
	// models
	else if (menu_tab == 3) {
		if (c_loki::menu_group(102, "custom material")) {
			c_loki::combo("base material", &config.chams.base, { "textured", "flat", "metallic", "glow", "ray" });
			c_loki::checkbox("keep original model", &config.chams.option3);
			c_loki::checkbox("wireframe", &config.chams.option1);
			c_loki::checkbox("pulse", &config.chams.option2);
		}
		if (c_loki::menu_group(134, "enemy model")) {
			c_loki::checkbox("active", &config.chams.enemy.enabled);
			c_loki::coloralpha("model", &config.chams.enemy.color);

			c_loki::checkbox("two pass", &config.chams.enemy.xqz);
			c_loki::coloralpha("model", &config.chams.enemy.xqz_color);

			c_loki::combo("material", &config.chams.enemy.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.enemy.type == 5 && config.chams.option2)
				c_loki::checkbox("preserve original", &config.chams.enemy.option1);
		}
		if (c_loki::menu_group(134, "backtrack model")) {
			c_loki::checkbox("active", &config.chams.backtrack.enabled);

			c_loki::coloralpha("model", &config.chams.backtrack.color);

			c_loki::checkbox("two pass", &config.chams.backtrack.xqz);
			c_loki::coloralpha("model", &config.chams.backtrack.xqz_color);
			c_loki::combo("material", &config.chams.backtrack.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.backtrack.type == 5 && config.chams.option2)
				c_loki::checkbox("preserve original", &config.chams.backtrack.option1);
		}
		c_loki::menu_next_column();
		if (c_loki::menu_group(185, "local model")) {
			c_loki::checkbox("active", &config.chams.local.enabled);
			c_loki::coloralpha("model", &config.chams.local.color);
			c_loki::combo("material", &config.chams.local.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			c_loki::combo("desync material", &config.chams.local.desync_type, { "off", "textured", "flat", "metallic", "glow", "ray", "custom" });
			c_loki::coloralpha("desync", &config.chams.local.desync_color);
			if (config.chams.local.type == 5 && config.chams.option2)
				c_loki::checkbox("preserve original", &config.chams.local.option1);
			if (config.chams.local.desync_type == 6 && config.chams.option2)
				c_loki::checkbox("preserve original desync", &config.chams.local.option2);
		}

		c_loki::menu_next_column();
		if (c_loki::menu_group(175, "hand model")) {
			c_loki::checkbox("active", &config.chams.hand.enabled);
			c_loki::coloralpha("model", &config.chams.hand.color);
			c_loki::combo("material", &config.chams.hand.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if(config.chams.hand.type == 5 && config.chams.option2)
				c_loki::checkbox("preserve original", &config.chams.hand.option1);
			c_loki::checkbox("no hands", &config.chams.hand.option2);
			c_loki::checkbox("no sleeves", &config.chams.hand.option3);
		}

		if (c_loki::menu_group(175, "weapon model")) {
			c_loki::checkbox("active", &config.chams.weapon.enabled);
			c_loki::coloralpha("model", &config.chams.weapon.color);
			c_loki::combo("material", &config.chams.weapon.type, { "textured", "flat", "metallic", "glow", "ray", "custom" });
			if (config.chams.weapon.type == 5 && config.chams.option2)
				c_loki::checkbox("preserve original", &config.chams.weapon.option1);
		}
	}
	// misc
	else if (menu_tab == 4){
	if (c_loki::menu_group(175, "general")) {
		c_loki::checkbox("watermark", &config.misc.watermark);
		c_loki::checkbox("round summary", &config.misc.on_screenplayers);
		c_loki::checkbox("ragdoll meme", &config.misc.gravity);
		c_loki::checkbox("save killfeed", &config.misc.preserve_feed);
		static std::vector<std::string> channels;
		static bool _____A_A_A_A_A = false;
		if (!_____A_A_A_A_A) {

			channels.push_back("off");
			for (auto x : g_RadioManager.stations) {
				channels.push_back(x.Name);
			}

			_____A_A_A_A_A = true;
		}


		c_loki::combo("radio", &config.misc.radio_channel, channels);
		c_loki::slider("radio volume", &config.misc.radio_volume, 0.f, 100.f, "");
	}

	if (c_loki::menu_group(175, "player hurt")) {
		c_loki::checkbox("hitmarker", &config.esp.hitmarker);
		c_loki::coloralpha("color", &config.esp.hitmarker_color, false);
		c_loki::checkbox("damage indicator", &config.esp.hitmarker_damage);
		c_loki::checkbox("hitsound", &config.esp.hitsound);
		c_loki::slider("hitsound volume", &config.esp.hitsound_volume, 0.f, 100.f, "");
		c_loki::checkbox("hit effect", &config.misc.hit_effect);
		c_loki::checkbox("kill effect", &config.misc.kill_effect);
	}
	if (c_loki::menu_group(-1, "movement")) {
		c_loki::checkbox("auto jump", &config.misc.knife);
	}
	c_loki::menu_next_column();
	if (c_loki::menu_group(122, "view")) {
		c_loki::slider("field of view", &config.misc.fov, 0.f, 150.f, "");
		c_loki::slider("v-field of view", &config.misc.fov_view, 0.f, 150.f, "");
		c_loki::hotkey("thirdperson", &config.misc.thirdperson_switch);
		c_loki::slider("distance", &config.misc.thirdperson_dist, 0.f, 200.f, "");
	}
	if (c_loki::menu_group(55, "identity")) {
		c_loki::checkbox("clantag spammer", &config.misc.clantag);
	}

	if (c_loki::menu_group(-1, "indicators")) {
		c_loki::checkbox("active", &config.misc.indicators);
		c_loki::checkbox("out of view", &config.esp.enemy.radar);
		c_loki::coloralpha("color", &config.esp.radar_color, false);

		c_loki::checkbox("antiaim", &config.misc.indicator_antiaim);
		c_loki::coloralpha("color", &config.misc.indicator_antiaimc, false);
		c_loki::checkbox("antiaim lines", &config.misc.indicator_antiaim_l);
	}
	c_loki::menu_next_column();
	if (c_loki::menu_group(200, "buybot")) {
		c_loki::checkbox("active", &config.misc.buy_bot);
		c_loki::combo("primary", &config.misc.buy_bot_primary, { "none", "auto", "scout", "awp" });
		c_loki::combo("secondary", &config.misc.buy_bot_secondary, { "none", "elites", "p250", "auto pistol", "heavy pistol" });
		c_loki::combo("armor", &config.misc.buy_bot_armor, { "none", "kevlar", "helmet + kevlar" });
		c_loki::checkbox("grenades", &config.misc.buy_bot_grenades);
		c_loki::checkbox("zeus", &config.misc.buy_bot_zeus);
		c_loki::checkbox("defuse kit", &config.misc.buy_bot_defuser);
	}
}














	// running combo to overlay everything! call at end plox
	c_loki::run_combo();
	c_loki::run_color();
	*/

