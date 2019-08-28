#include <Windows.h>
#include <tchar.h>
#include <ShlObj.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Mpr.lib")

#include "../X-Elite Client Stub/Utils.h"

bool vbox_reg_key_value();
bool vbox_reg_keys();
bool vbox_files();
BOOL vbox_dir();

BOOL vbox_check_mac();
bool vbox_devices();
BOOL vbox_window_class();
BOOL vbox_network_share();
bool vbox_processes();
