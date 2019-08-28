#include "sdk/misc.h"
#include "ProperColor.h"

#include "ImGUI/imgui.h"

#include <vector>

class cMenu {
public:
	void Initialize();

	void ColorPicker(const char * name, Color & item, int cp);



	void CreateGroup(const char* title, int height);
	void EndGroup(bool next = false, bool last = false);
	void SubTabs(std::vector<std::string> tabs, ImFont * font, int & index);
	void Run();
}; extern cMenu* Menu;

namespace menuInfo {
	extern bool menuActive;
	extern ImVec2 menuPosition;
	extern int currentTab;
}

enum retarcolors {
	COLOR_THEME = 0,
	COLOR_CBOX,
	COLOR_CFILLEDBOX,

};