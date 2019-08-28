#include "bobhack_main.h"
#include "hooks/c_hooks.h"
#include "hooks/idirect3ddevice9.h"
#include "menu/c_menu.h"
#include <thread>

using namespace std::chrono_literals;

rifk_loader_info g_loader_info{};
DWORD g_image_base = 0x0;

c_rifk::c_rifk()
{
	c_menu::instance();
	c_netvar::instance();
	c_hooks::run();
}

#define ERR_TITLE ("DEBUGGING")

inline __declspec(noinline) void THROW_ERROR(const char* errorcode)
{
	MessageBox(NULL, errorcode, ERR_TITLE, MB_OK);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
#ifndef RELEASE
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
#endif
		g_image_base = (DWORD)hinstDLL;

		
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)playback_loop, 0, 0, 0); // this works :shrug:
		/*HANDLE thread;

		syscall(NtCreateThreadEx)(&thread, THREAD_ALL_ACCESS, nullptr, current_process,
			nullptr, nullptr, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr);

		CONTEXT context;
		context.ContextFlags = CONTEXT_FULL;
		syscall(NtGetContextThread)(thread, &context);

		context.Eax = reinterpret_cast<uint32_t>(&playback_loop);

		syscall(NtSetContextThread)(thread, &context);
		syscall(NtResumeThread)(thread, nullptr);*/

		if (lpReserved)
			g_loader_info = *(rifk_loader_info*)lpReserved;

		

#ifndef RELEASE
		if (strlen(g_loader_info.loader_path) < 2)
		{
			char szDllPath[MAX_PATH];
			GetModuleFileNameA(hinstDLL, szDllPath, MAX_PATH);
			std::string::size_type pos = std::string(szDllPath).find_last_of("\\/");
			strcpy_s(g_loader_info.loader_path, std::string(szDllPath).substr(0, pos).c_str());
		}
#endif
	}

	return TRUE;
}
