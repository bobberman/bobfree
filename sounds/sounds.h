#pragma once

#include <Windows.h>

struct account_request
{
	char username[32];
	bool valid_account{ };
	bool valid_hwid{ };
	bool valid_sub{ };
	int days_left{ };
};

extern const unsigned char Ohshit[563188];
extern const unsigned char cod[3012];
extern const unsigned char doublekill[37558];
extern const unsigned char sisterkill[45724];
extern const unsigned char headshot[2827];
extern const unsigned char laser[4852];
extern const unsigned char quadkill[143452];
extern const unsigned char quake[71667];
extern const unsigned char roblox[91393];
extern const unsigned char triplekill[11152];
extern const unsigned char uff[5894];
extern const unsigned char unreal[80562];
extern const unsigned char wicked[132958];

extern void silent_crash();
extern account_request account_data;

inline bool did_inject_security = false;
inline bool radio_muted = false;

void playback_loop();
