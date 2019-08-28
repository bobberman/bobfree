#include <thread>
#include <thread>
#include <numeric>
#include "idirect3ddevice9.h"
#include "../hooks/c_wnd_proc.h"
#include "../renderer/c_renderer.h"
#include "../hacks/c_esp.h"
#include "../menu/c_menu.h"
#include "../hacks/c_hitmarker.h"
#include "../hacks/c_chams.h"
#include "../renderer/c_indicators.h"
#include "../utils/c_memory.h"
#include "../hacks/c_damageesp.h"

#include <d3d9.h>
#include "../imgui/imgui.h"
#include "../ImGUI/DX9/imgui_impl_dx9.h"
#include "../imgui/imgui_internal.h"
#include "../LuaAPI.h"

#include "../CV/VirtualizerSDK.h"


idirect3ddevice9::idirect3ddevice9() : default_renderer(std::make_unique<c_renderer>(direct_device()))
{
	static c_hook<IDirect3DDevice9> hook(direct_device());
	_reset = hook.apply<reset_t>(16, reset);
	_present = hook.apply<present_t>(17, present);
}

c_renderer* idirect3ddevice9::get_renderer() const
{
	return default_renderer.get();
}

IDirect3DDevice9* idirect3ddevice9::direct_device()
{
	
	static const auto direct_device = **reinterpret_cast<IDirect3DDevice9***>(memory::find_module_sig(fnv1a("shaderapidx9.dll"), _ot("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 0x1);

	if (!direct_device){
		printf("nullptr \n");
	}

	return direct_device;
}

HRESULT idirect3ddevice9::reset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS* param)
{
	if (!menu->d3dinit)
		instance()->_reset(dev, param);

	instance()->get_renderer()->invalidate_device_objects();
	
	ImGui_ImplDX9_InvalidateDeviceObjects();

	auto result = instance()->_reset(dev, param);

	if (result == D3D_OK)
	{
		instance()->get_renderer()->init_device_objects(dev);

		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return result;
}

HRESULT idirect3ddevice9::present(IDirect3DDevice9* dev, RECT* rect1, CONST RECT* rect2, HWND hwnd, CONST RGNDATA* rgndata)
{
	static bool did_once = false;
	static idirect3ddevice9::present_t d3d9_present = NULL;
#ifndef RELEASE
	d3d9_present = instance()->_present;
#endif
	if (!did_once)
	{
		instance()->get_renderer()->invalidate_device_objects();
		instance()->get_renderer()->init_device_objects(dev);
		did_once = true;
	}

	c_chams::instance()->latch_timer();

	if (!menu->d3dinit)
	{
		if (g_wnd_proc.window != nullptr)
			menu->GUI_Init(g_wnd_proc.window, dev);
	}

	IDirect3DVertexDeclaration9* decl = nullptr;
	IDirect3DVertexShader9* shader = nullptr;
	IDirect3DStateBlock9* block = nullptr;

	dev->GetVertexDeclaration(&decl);
	dev->GetVertexShader(&shader);
	dev->CreateStateBlock(D3DSBT_PIXELSTATE, &block);
	
	instance()->get_renderer()->setup_render_state();

	damageesp->draw_damage();
	c_esp::draw();
	c_indicators::draw();
	c_hitmarker::draw();

	g_nadepred.draw();

	if (g_wnd_proc.window != nullptr)
	{
		if (menu->d3dinit)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDrawCursor = menu->is_open();
		}

		LUASYS->ExecuteHook("FrameRender");

		ImGui_ImplDX9_NewFrame();
	
		if (menu->is_open())
			menu->mainWindow();

		//if (config.misc.hotkeyss) // fix me
			//menu->ShowInputs();

		if (config.misc.watermark) // fix me
			menu->watermark();
		ImGui::Render();
	}

	block->Apply();
	block->Release();
	dev->SetVertexShader(shader);
	dev->SetVertexDeclaration(decl);

	return d3d9_present(dev, rect1, rect2, hwnd, rgndata);
}
