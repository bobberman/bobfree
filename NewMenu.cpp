#include "NewMenu.h"
#include "BrokenConfiguration.h"

cMenu* Menu;
ImFont* Icon;
ImFont* regularFont;
ImFont* TitleFont;

bool menuInfo::menuActive = true;
ImVec2 menuInfo::menuPosition;
int menuInfo::currentTab;

float lerp(float t, float a, float b) {
	return (1 - t)*a + t * b;
}

static bool gay2;
void cMenu::Initialize() {
	ImGuiStyle& Style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();

	Style.Colors[ImGuiCol_Text] = ImColor(180, 180, 180, 255);
	Style.Colors[ImGuiCol_WindowBg] = ImColor(50, 50, 50, 255);
	Style.Colors[ImGuiCol_TitleBg] = ImColor(60, 60, 60, 255);
	Style.Colors[ImGuiCol_TitleBgActive] = ImColor(60, 60, 60, 255);
	Style.Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 255);
	Style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	Style.Colors[ImGuiCol_SliderGrabActive] = ImColor(185, 185, 185, 255);
	Style.Colors[ImGuiCol_FrameBgHovered] = ImColor(125, 125, 125, 255);
	Style.Colors[ImGuiCol_FrameBgActive] = ImColor(125, 125, 125, 255);
	Style.Colors[ImGuiCol_ChildWindowBg] = ImColor(50, 50, 50, 255);

	//Style.Colors[ImGuiCol_CloseButton] = ImColor(120, 120, 200, 255);
	//Style.Colors[ImGuiCol_CloseButtonActive] = ImColor(Style.Colors[ImGuiCol_CloseButton].x - 30, Style.Colors[ImGuiCol_CloseButton].y - 30, Style.Colors[ImGuiCol_CloseButton].z - 30, 255.f);

	Style.Colors[ImGuiCol_Button] = ImColor(70, 70, 70, 255);
	Style.Colors[ImGuiCol_ButtonActive] = ImColor(90, 90, 90, 255);
	Style.Colors[ImGuiCol_ButtonHovered] = ImColor(90, 90, 90, 255);

	Style.Alpha = 1;
	Style.WindowRounding = 0;
	Style.FrameRounding = 0;
	Style.ScrollbarRounding = 0;
	Style.ItemSpacing = ImVec2(4, 4);


	Icon = io.Fonts->AddFontFromFileTTF("C:\\LOKIRED\\resources\\icons.ttf", 22);
	regularFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 12);
	TitleFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 16);

	//ImConsole::Init();
}
ImFont* Font;

float clamp(float in, float min, float max) {
	if (in > max) return max;
	if (in < min) return min;
	return in;
}


bool set_hsvv[256];
float hsvv[256];
Color colorr[256];
void cMenu::ColorPicker(const char* name, Color& item, int cp) {


	float hsvtest[3] = { (float)item.r, (float)item.g, (float)item.b };
	colorr[cp].ToHSV(hsvtest[0], hsvtest[1], hsvtest[2]);
	if (!set_hsvv[cp]) {
		hsvv[cp] = colorr[cp].r;
		set_hsvv[cp] = true;
	}

	//ImGui::DragFloat((ImVec4)ImColor(item.r, item.g, item.b, 255), name, &hsvv[cp], 0.002f, 0.f, 1.f);
	colorr[cp].FromHSV(clamp(hsvv[cp], 0.f, 1.f), 1.f, 1.f);
	item = colorr[cp];
}





static std::vector<std::string> tabicons = { "A","C","D", "G", "F", "I" };
static std::vector<std::string> tabtext = { "AIMBOT","ANTIAIM","VISUALS", "MISC", "CONSOLE", "SKINS" };

static int id = 0;
static int max_height = 421;
static int ava_height = 421;
int off = 0;
void cMenu::CreateGroup(const char* title, int height) {

	int using_height = height;

	if (height <= 0) using_height = ava_height;
	if (height >= ava_height) using_height = ava_height;

	ava_height -= using_height + 4;




	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
	ImGui::BeginChild(title, ImVec2(-1, using_height), true);
	ImGui::PushID(id);

	auto ps = ImGui::GetWindowPos();
	auto dl = ImGui::GetWindowDrawList();
	auto cp = ImGui::GetWindowSize();
	ImGui::PushClipRect(ImVec2(ps.x, ps.y - 8), ImVec2(ps.x + cp.x, ps.y + cp.y), false);
	ImGui::PushFont(regularFont);
	auto sz = ImGui::GetFont()->CalcTextSizeA(12.f, FLT_MAX, 0.f, title);
	dl->AddLine(ImVec2(ps.x + 2, ps.y), ImVec2(ps.x + 7 + sz.x, ps.y), ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg]));
	dl->AddText(ImVec2(ps.x + 5, ps.y - 7), ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), title);

	ImGui::PopFont();
	ImGui::PopClipRect();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

}

void cMenu::EndGroup(bool next, bool last) {
	ImGui::PopID();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	id++;

	if (next) {
		ava_height = max_height;
		if (!last)
			ImGui::NextColumn();
	}
}
float slider_pos[128]; // no need for this much but ye
float lslider_pos[128]; // no need for this much but ye
int subtab_offset = 0;
void cMenu::SubTabs(std::vector<std::string> tabs, ImFont* font, int &index) {
	lslider_pos[subtab_offset] = lerp(0.1f, lslider_pos[subtab_offset], slider_pos[subtab_offset]);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::BeginChild("##Subtabs", ImVec2(ImGui::GetWindowSize().x, 26), false);
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 11, ImGui::GetWindowPos().y + 2), /*ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_CloseButton])*/ 0xFF00FF, 0.f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	int e = 0;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.4f));

	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(40, 40, 40, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(30, 30, 30, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(30, 30, 30, 255));


	for (const auto &it : tabs) {
		int pop = 1;
		if (e == index) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1)/*ImGui::GetStyle().Colors[ImGuiCol_CloseButton]*/);
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(30, 30, 30, 255));
			slider_pos[subtab_offset] = ImGui::GetCursorPosX();
			pop = 2;
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(180, 180, 180, 180));
		}
		if (ImGui::Button(it.c_str(), ImVec2((ImGui::GetWindowSize().x - 11) / tabs.size(), 22)))
			index = e;


		ImGui::PopStyleColor(pop);
		ImGui::SameLine();
		e++;
	}
	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(2);
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 24), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 11, ImGui::GetWindowPos().y + 26), ImGui::GetColorU32((ImVec4)ImColor(30, 30, 30, 200)), 0.f);
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x + lslider_pos[subtab_offset], ImGui::GetWindowPos().y + 24), ImVec2(ImGui::GetWindowPos().x + lslider_pos[subtab_offset] + ((ImGui::GetWindowSize().x - 11) / tabs.size()), ImGui::GetWindowPos().y + 26), /*ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_CloseButton])*/0xFF00FF, 0.f);
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
	subtab_offset++;
}

void cMenu::Run() {

	static float alpha = 0.f;


	if (menuInfo::menuActive && alpha < 1.f)
		alpha = lerp(0.05f, alpha, 1.f);


	if (!menuInfo::menuActive && alpha > 0.f)
		alpha = lerp(0.05f, alpha, 0.f);


	bool show = alpha > 0.f;
	ImGui::GetStyle().Alpha = alpha;
	if (!show)
		return;

	static float sliderpos = 0.f;

	ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(650, 500));
	id = 0;
	subtab_offset = 0;

	if (ImGui::Begin("##Main", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse /*| ImGuiWindowFlags_ShowBorders*/ | ImGuiWindowFlags_NoTitleBar)) {
		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, (ImVec4)ImColor(30, 30, 30, 255));
		ImGui::BeginChild("##Sub", ImVec2(ImGui::GetWindowSize().x - 16, ImGui::GetWindowSize().y - 16), true);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 3), ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_CloseButton]), 0.f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
		// TABS
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::BeginChild("##Tabs", ImVec2(ImGui::GetWindowSize().x + 12, 32), false);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImColor(50, 50, 50, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(40, 40, 40, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(40, 40, 40, 255));
		int i = 0;
		for (const auto &it : tabicons) {
			auto sz = 30;
			int pop = 1;
			std::string t = (i == menuInfo::currentTab) ? tabtext[i] : it;
			if (i == menuInfo::currentTab) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_CloseButton]);
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(40, 40, 40, 255));
				ImGui::PushFont(TitleFont);
				sliderpos = ImGui::GetCursorPosX() + 5;
				pop = 2;
			}
			else {
				ImGui::PushFont(Icon);
				ImGui::PushStyleColor(ImGuiCol_Text, ImColor(180, 180, 180, 180));
			}


			if (ImGui::Button(t.c_str(), ImVec2((ImGui::GetWindowSize().x - 16) / tabicons.size(), sz))) menuInfo::currentTab = i;


			ImGui::PopStyleColor(pop);
			ImGui::PopFont();
			ImGui::SameLine();
			i++;
		}
		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PushFont(regularFont);
		// en d of tabs
		//
		static float lerpedsliderpos = 0.f;
		lerpedsliderpos = Visuals->lerp(0.1f, lerpedsliderpos, sliderpos);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 37), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 37 + 3), ImGui::GetColorU32(ImColor(180, 180, 180, 30)), 0.f);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ImGui::GetWindowPos().x + lerpedsliderpos, ImGui::GetWindowPos().y + 37), ImVec2(ImGui::GetWindowPos().x + lerpedsliderpos + ((ImGui::GetWindowSize().x - 16) / tabicons.size()), ImGui::GetWindowPos().y + 37 + 3), ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_CloseButton]), 0.f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.5f);
		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(30, 30, 30, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::BeginChild("##MainMainToomanyfuckingchildren", ImVec2(ImGui::GetWindowSize().x - 16, ImGui::GetWindowSize().y - 59), false);

		// 4 = console
		if (menuInfo::currentTab != 4) {
			ImGui::Columns(3, NULL, false);
		}


		switch (menuInfo::currentTab) {

			// visuals
		case 2: {
			CreateGroup("Players", -1);
			ImGui::Checkbox("Name", &Configuration::Visuals::Player::Name);
			ImGui::Checkbox("Box", &Configuration::Visuals::Player::Box); this->ColorPicker("CBox", Configuration::Visuals::Player::CBox, COLOR_CBOX);
			ImGui::Checkbox("Filled box", &Configuration::Visuals::Player::FilledBox); this->ColorPicker("CFilledBox", Configuration::Visuals::Player::CFilledBox, COLOR_CFILLEDBOX);



			ImGui::Checkbox("Health", &Configuration::Visuals::Player::Health);
			ImGui::Checkbox("Armor", &Configuration::Visuals::Player::Armor);
			ImGui::Checkbox("Weapon", &Configuration::Visuals::Player::Weapon);
			ImGui::Checkbox("Ammo", &Configuration::Visuals::Player::Ammo);


			EndGroup(true);

			CreateGroup("Models", -1);
			static int model_target = 0;
			SubTabs(std::vector<std::string>{"Enemy", "Team", "Local"}, regularFont, model_target);
			ImGui::Checkbox("Chams enabled", &Configuration::Visuals::Player::Chams::Enabled);
			switch (model_target) {
			case 0: {
				ImGui::Checkbox("Enemy enabled", &Configuration::Visuals::Player::Chams::Enemy::Enabled);
				ImGui::Checkbox("XQZ", &Configuration::Visuals::Player::Chams::Enemy::XQZ);
			} break;
			case 1: {
				ImGui::Checkbox("Team enabled", &Configuration::Visuals::Player::Chams::Team::Enabled);
				ImGui::Checkbox("XQZ", &Configuration::Visuals::Player::Chams::Team::XQZ);
			} break;
			}



			EndGroup(true);

			CreateGroup("Undefined 3", 100);

			EndGroup();

			CreateGroup("View", 100);
			ImGui::SliderInt("Field of view", &Configuration::Visuals::View::Fov, 0, 45);
			ImGui::SliderInt("Viewmodel field of view", &Configuration::Visuals::View::viewmodelFov, 0, 60);

			EndGroup();


			CreateGroup("Other", -1);
			ImGui::Checkbox("Crosshair", &Configuration::Visuals::View::crosshair);

			EndGroup(true, true);
		} break;
			// misc/cfg
		case 3: {

			CreateGroup("Configuration", -1);
			/*ImGui::RealText("Menu theme");*/
			this->ColorPicker("Theme", Configuration::Menu::Theme, COLOR_THEME);
			/*if (ImGui::Button("Save"))
				g_ConfigMNGR.Save();
			if (ImGui::Button("Load"))
				g_ConfigMNGR.Load();*/

			EndGroup(true);
		} break;
		case 4: {

			//ImConsole::Run();

			break;
		}
		};











		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::EndChild();
		ImGui::PopStyleColor();
	} ImGui::End();


	auto theme = Configuration::Menu::Theme;
	//ImGui::GetStyle().Colors[ImGuiCol_CloseButton] = ImColor(theme.r, theme.g, theme.b, 255);
}