#include "ImConsole.h"

#include <vector>
#include "LuaAPI.h"

ImConsoleTextItem::ImConsoleTextItem(std::string str, ImVec4 Color) {
	this->m_String	= str;
	this->m_Color	= Color;
}

ImConsoleTextItem::~ImConsoleTextItem() {

}

std::string ImConsoleTextItem::GetString() {
	return this->m_String;
}

ImVec4 ImConsoleTextItem::GetColor() {
	return this->m_Color;
}

void ImConsole::Init() {
	auto& io = ImGui::GetIO();
	this->m_pConsoleFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 12);
	this->m_pConsoleFont2 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 10);

	LUASYS->Setup();
}

void ImConsole::Run() {
	if (!m_pConsoleFont) {
		ImGui::TextColored(ImVec4(1.f, 0.75f, 0.75f, 1.f), "Font1 not initialized!");
		return;
	}

	if (!m_pConsoleFont2) {
		ImGui::TextColored(ImVec4(1.f, 0.75f, 0.75f, 1.f), "Font2 not initialized!");
		return;
	}

	ImGui::PushItemWidth(500);

	ImGui::PushID(821094234);
	ImGui::PushFont(m_pConsoleFont2);

	if (ImGui::InputText("", this->m_szCmdBuffer, 128, ImGuiInputTextFlags_EnterReturnsTrue)) {

		auto newString = std::string(std::string(std::string("> ") + this->m_szCmdBuffer + std::string("\n")));

		this->m_Items.push_back(ImConsoleTextItem(newString, CONCLR_NORMAL));
		
		if (!ImConsole::HandleCommand(std::string(this->m_szCmdBuffer))) {
			this->m_Items.push_back(ImConsoleTextItem("[ERROR] Could not find command.", CONCLR_ERROR));
		}

		for (int i = 0; i < 128; i++) this->m_szCmdBuffer[i] = '\0';
	}
	ImGui::SameLine();
	ImGui::Checkbox("Show Lua Editor", &LUASYS->m_bShowLuaEditor);

	ImGui::PopFont();
	ImGui::PopID();

	ImGui::PushFont(m_pConsoleFont);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
	ImGui::BeginChild("ScrollingShite");

	for (auto Item : this->m_Items) {
		ImGui::PushStyleColor(ImGuiCol_Text, Item.GetColor());
		ImGui::TextWrapped(Item.GetString().c_str());
		ImGui::PopStyleColor();
	}

	if (this->m_bAddedToBuffer) {
		ImGui::SetScrollHere(1.0f); // scroll to the bottom of the frame

		this->m_bAddedToBuffer = false;
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::PopItemWidth();
}

bool ImConsole::HandleCommand(std::string cmdText) {
	auto shit = StrSplit(cmdText, " ");

	if (strcmp(shit[0].c_str(), "lua_run") == 0) {
		// add 1 to compensate for the space
		auto luacode = cmdText.substr(strlen("lua_run") + 1, cmdText.size());

		LUASYS->RunString(luacode);
		
		return true;
	}
	else if (strcmp(shit[0].c_str(), "lua_init") == 0) {
		LUASYS	->Setup();
		CONSOLE	->AddToBuffer("Lua system reloaded\n");

		return true;
	}
	else {
		return false;
	}

	return false;
}

void ImConsole::AddToBuffer(std::string str) {
	m_Items.push_back(ImConsoleTextItem(str, CONCLR_NORMAL));

	this->m_bAddedToBuffer = true;
}

std::vector<std::string> StrSplit(const std::string& str, const char* delim) {
	std::vector<std::string> res;
	char* pTempStr = _strdup(str.c_str());
	char* context = NULL;
	char* pWord = strtok_s(pTempStr, delim, &context);
	while (pWord != NULL) {
		res.push_back(pWord);
		pWord = strtok_s(NULL, delim, &context);
	}

	free(pTempStr);

	return res;
}