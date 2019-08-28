#pragma once

#include "utils/c_singleton.h"

#include <vector>
#include <string>

struct RadioStation {
	std::string Name;
	std::string Url;
};

class RadioManager
{
public:
	RadioManager();

	~RadioManager();

	std::vector<RadioStation> stations;
};

extern RadioManager g_RadioManager;