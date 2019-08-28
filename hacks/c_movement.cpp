#include "c_movement.h"
#include "../utils/math.h"
#include "../sdk/c_cvar.h"
#include "c_prediction_system.h"

void c_movement::run(c_cs_player* local, c_user_cmd* cmd)
{
	bhop(local, cmd);
}

void c_movement::autostrafe(c_cs_player* local, c_user_cmd* cmd)
{



		const auto vel = local->get_velocity().length2d();

		if (vel < 1.f)
			return;

		else if (cmd->mousedx > 1 || cmd->mousedx < -1)
			cmd->sidemove = cmd->mousedx < 0.f ? -450.f : 450.f;
		else
		{
			cmd->forwardmove = std::clamp(10000.f / vel, -450.0f, 450.0f);

			cmd->sidemove = cmd->command_number % 2 == 0 ? -450.f : 450.f;
		}
	
};

void c_movement::bhop(c_cs_player* local, c_user_cmd* cmd)
{
	

	static const auto sv_autobunnyhopping = cvar()->find_var(_("sv_autobunnyhopping"));
	static const auto sv_enablebunnyhopping = cvar()->find_var(_("sv_enablebunnyhopping"));

	if (config.misc.knife)
	{
		sv_autobunnyhopping->set_value(1);
		sv_enablebunnyhopping->set_value(1);
	}
	else
	{
		sv_autobunnyhopping->set_value(0);
		sv_enablebunnyhopping->set_value(0);
		return;
	}

	static auto last_jumped = false;
	static auto should_fake = false;

	const auto move_type = local->get_move_type();
	const auto flags = local->get_flags();

	if (move_type != c_cs_player::movetype_ladder && move_type != c_cs_player::movetype_noclip &&
		!(flags & c_cs_player::in_water))
	{
		if (!last_jumped && should_fake)
		{
			should_fake = false;
			cmd->buttons |= c_user_cmd::jump;
		}
		else if (cmd->buttons & c_user_cmd::jump)
		{
			autostrafe(local, cmd);

			if (flags & c_cs_player::on_ground)
			{
				last_jumped = true;
				should_fake = true;
			}
			else
			{
				cmd->buttons &= ~c_user_cmd::jump;
				last_jumped = false;
			}
		}
		else
		{
			last_jumped = false;
			should_fake = false;
		}
	}
}



// now we gotta do retarded definitions n shit
inline float sseSqrt(float x)
{
	float	root = 0.f;
	__asm
	{
		sqrtss		xmm0, x
		movss		root, xmm0
	}
	return root;
}
static c_vector3d VectorAngles(const c_vector3d& start, const c_vector3d& end)
{
	float pitch = 0.f;
	float yaw = 0.f;

	c_vector3d forward = end - start;

	if ((forward[1] == 0.0f && forward[0] == 0.0f) == 1)
	{
		if (forward[2] <= 0.0f)
			pitch = 90.0f;
		else
			pitch = 270.0f;
		yaw = 0.0f;
	}
	else
	{
		yaw = atan2(forward[1], forward[0]) * 57.29577951308232f;
		if (yaw < 0.0f)
			yaw = yaw + 360.0f;

		pitch = (atan2(-forward[2],
			sseSqrt((float)(forward[1] * forward[1]) + (float)(forward[0] * forward[0]))) * 57.29577951308232f);

		if (pitch < 0.0f)
			pitch = pitch + 360.0f;
	}

	c_vector3d angles = c_vector3d();
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.f;
	angles.Sanitize();
	return angles;
}
template <typename t>
t clamp(t x, t min, t max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
};

c_vector3d peek_start;
bool should_unpeek = false;
bool set_peek_start = false;

bool c_movement::yeetingback() {
	

	return false;
}

void c_movement::auto_peek(c_cs_player* local, float& forwardmove, float& sidemove, c_user_cmd* cmd)
{
	
}




// defs for quick peek

void c_vector3d::Clamp() {
	clamp(x, -89.0f, 89.0f);
	clamp(y, -180.0f, 180.0f);
	z = 0.0f;
}
void c_vector3d::Normalize() {
	while (x > 180.f)
		x -= 360.f;

	while (x < -180.f)
		x += 360.f;

	while (y > 180.f)
		y -= 360.f;

	while (y < -180.f)
		y += 360.f;
}
void c_vector3d::Sanitize() {
	//safety
	if (!isfinite(x) || isnan(x) || isinf(x))
		x = 0.0f;

	if (!isfinite(y) || isnan(y) || isinf(y))
		y = 0.0f;

	//normalize
	Normalize();

	//clamp
	Clamp();
}