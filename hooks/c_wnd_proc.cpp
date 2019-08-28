#include "c_wnd_proc.h"
#include "../menu/c_menu.h"

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

c_wnd_proc g_wnd_proc;

void c_wnd_proc::hook()
{
	window = FindWindowA(_("Valve001"), nullptr);
	_wnd_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG>(wnd_proc)));
}

LRESULT c_wnd_proc::wnd_proc(const HWND wnd, const UINT msg, const WPARAM w_param, const LPARAM l_param)  // NOLINT
{
	if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(l_param & previous_state_mask))
		hotkeys->process_input(w_param);

	if ((msg == WM_KEYUP || msg == WM_SYSKEYUP) && (l_param & previous_state_mask))
		hotkeys->process_stop_input(w_param);

	if (msg == WM_XBUTTONDOWN && !(l_param & previous_state_mask))
		hotkeys->process_input(GET_XBUTTON_WPARAM(w_param) - 1 ? VK_XBUTTON2 : VK_XBUTTON1);

	if (msg == WM_XBUTTONUP && !(l_param & previous_state_mask))
		hotkeys->process_stop_input(GET_XBUTTON_WPARAM(w_param) - 1 ? VK_XBUTTON2 : VK_XBUTTON1);

	if (msg == WM_MBUTTONDOWN && !(l_param & previous_state_mask))
		hotkeys->process_input(VK_MBUTTON);

	if (msg == WM_MBUTTONUP && !(l_param & previous_state_mask))
		hotkeys->process_stop_input(VK_MBUTTON);

	if (menu->d3dinit && menu->is_open() && ImGui_ImplDX9_WndProcHandler(wnd, msg, w_param, l_param) && !menu->input_shouldListen)
		return true;

	return CallWindowProc(_wnd_proc, wnd, msg, w_param, l_param);
}
