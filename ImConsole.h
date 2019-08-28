#pragma once

#include <string>
#include <vector>
#include "utils/c_singleton.h"
#include "ImGUI/imgui.h"

/* recoded at ~8:30 am to be more clean due to me getting pissed at it not being class based, same for lua api */

#define CONCLR_ERROR (ImVec4)ImColor(255, 100, 100)
#define CONCLR_NORMAL (ImVec4)ImColor(255, 255, 255)

class ImConsoleTextItem {
public:

	ImConsoleTextItem(std::string str, ImVec4 Color);
	~ImConsoleTextItem();

	std::string GetString();
	ImVec4 GetColor();

private:

	std::string		m_String;
	ImVec4			m_Color;
};

class ImConsole : public c_singleton<ImConsole> {
public:

	void Init();

	void Run();

	bool HandleCommand(std::string cmdText);

	void AddToBuffer(std::string str);

	ImFont* m_pConsoleFont;
	ImFont* m_pConsoleFont2;

private:

	bool m_bInitialised = false;

	char	m_szCmdBuffer[128];
	bool	m_bAddedToBuffer = false;
	
	std::vector<ImConsoleTextItem> m_Items;
};

#define CONSOLE ImConsole::instance()

void Lua_Print(const char* str);
std::vector<std::string> StrSplit(const std::string& str, const char* delim);