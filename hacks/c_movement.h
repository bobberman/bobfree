#pragma once

#include "../includes.h"

class c_movement
{
public:
	static void run(c_cs_player* local, c_user_cmd* cmd);
	static bool yeetingback();
private:
    static void bhop( c_cs_player* local, c_user_cmd* cmd );
    static void autostrafe( c_cs_player* local, c_user_cmd* cmd );

	
	
	
	
	

	

	static void auto_peek(c_cs_player* local, float& forwardmove, float& sidemove, c_user_cmd* cmd);
};
