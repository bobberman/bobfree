#pragma once

#include "c_glow_object_manager.h"
#include "../utils/c_memory.h"

c_glow_object_manager* c_glow_object_manager::get()
{
	static const auto glowobject = *(c_glow_object_manager**)(sig("client_panorama.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 0x3);
	return glowobject;
}
