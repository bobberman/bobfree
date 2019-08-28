#pragma once

#include "c_font.h"
#include "../sdk/c_vector2d.h"
#include "../sdk/c_vector3d.h"
#include "../sdk/matrix.h"
#include "../sdk/c_color.h"
#include "../security/fnv1a.h"
#include <unordered_map>

class CD3DFont;

class c_renderer
{
	struct vertex {
		float x, y, z, rhw;
		uint32_t color;
	};

public:
	explicit c_renderer(IDirect3DDevice9* dev);

	void line(c_vector2d from, c_vector2d to, c_color color) const;

	void rect(c_vector2d from, c_vector2d size, c_color color) const;
	void rect_linear_gradient(c_vector2d from, c_vector2d size, c_color color1, c_color color2, bool horizontal) const;
	void rect_full_linear_gradient(c_vector2d from, c_vector2d size, c_color color1, c_color color2, c_color color3, c_color color4) const;
	void loki_gradient(const c_vector2d from, const c_vector2d size, const c_color color1, const c_color color2, const c_color color3, const c_color color4) const;
	void rect_filled(c_vector2d from, c_vector2d size, c_color color) const;
	void rect_filled_linear_gradient(c_vector2d from, c_vector2d size, c_color color1, c_color color2, bool horizontal = false) const;
	void rect_filled_radial_gradient(c_vector2d from, c_vector2d size, c_color color1, c_color color2);
	void rect_filled_diamond_gradient(c_vector2d from, c_vector2d size, c_color color1, c_color color2) const;

	void parallelogram(c_vector2d from, c_vector2d size, c_color color, uint8_t side = 0, float radius = 8.f) const;
	void parallelogram_filled_linear_gradient(c_vector2d from, c_vector2d size, c_color color1, c_color color2,
		uint8_t side = 0, bool horizontal = false, float radius = 8.f) const;

	void triangle(c_vector2d pos1, c_vector2d pos2, c_vector2d pos3, c_color color) const;
	void triangle_linear_gradient(c_vector2d pos1, c_vector2d pos2, c_vector2d pos3, c_color color1,
		c_color color2, c_color color3) const;
	void triangle_filled(c_vector2d pos1, c_vector2d pos2, c_vector2d pos3, c_color color) const;
	void triangle_filled_linear_gradient(c_vector2d pos1, c_vector2d pos2, c_vector2d pos3, c_color color1,
		c_color color2, c_color color3) const;

	void circle(c_vector2d center, float radius, c_color color);
	void circle_filled(c_vector2d center, float radius, c_color color);
	void circle_filled_radial_gradient(c_vector2d center, float radius, c_color color1, c_color color2);

	void text(c_vector2d pos, const char* str, c_color color, uint32_t font = default_font, uint8_t flags = 0);
	c_vector2d get_text_size(const char* str, uint32_t font);

	void ball(c_vector3d center, float radius, matrix3x4 transform, c_color col, viewmatrix& matrix);

	void create_texture(void* src, uint32_t size, LPDIRECT3DTEXTURE9* texture) const;
	void create_sprite(LPD3DXSPRITE* sprite) const;
	static void image(c_vector2d position, LPDIRECT3DTEXTURE9 texture, LPD3DXSPRITE sprite, float scl = 1.f, float alpha = 1.f);

	inline void scale(c_vector2d& vec) const;
	c_vector2d get_center() const;

	viewmatrix& world_to_screen_matrix();
	inline bool screen_transform(const c_vector3d& in, c_vector2d& out, viewmatrix& matrix) const;
	inline bool world2screen(c_vector3d In, c_vector3d & Out, matrix3x4t matrix);
	
	inline bool is_on_screen(const c_vector3d& in, float width, viewmatrix& matrix) const;

	void limit(rectangle rect) const;
	rectangle get_limit() const;
	void reset_limit() const;

	void Fonts(HWND window, IDirect3DDevice9 * pDevice);
	void refresh_viewport();
	void init_device_objects(IDirect3DDevice9* dev);
	void invalidate_device_objects();
	void setup_render_state() const;
	void register_reset_handler(std::function<void()> handler);

private:
	static constexpr auto points = 64;
	static constexpr auto points_sphere_latitude = 16;
	static constexpr auto points_sphere_longitude = 24;
	static constexpr auto default_font = fnv1a("pro13");

	IDirect3DDevice9* dev {};
	D3DVIEWPORT9 port {};
	std::vector<c_vector2d> lookup_table;
	std::vector<c_vector3d> lookup_sphere;
	std::unordered_map<uint32_t, c_font> fonts;
	std::unordered_map<uint32_t, CD3DFont*> betterfonts;
	std::vector<std::function<void()>> reset_handlers;

	void build_lookup_table();
};

void c_renderer::scale(c_vector2d& vec) const
{
	vec.x = (vec.x + 1.f) * port.Width / 2.f;
	vec.y = (-vec.y + 1.f) * port.Height / 2.f;
}

bool c_renderer::screen_transform(const c_vector3d& in, c_vector2d& out, viewmatrix& matrix) const
{
	out.x = matrix[0][0] * in.x + matrix[0][1] * in.y + matrix[0][2] * in.z + matrix[0][3];
	out.y = matrix[1][0] * in.x + matrix[1][1] * in.y + matrix[1][2] * in.z + matrix[1][3];

	const auto w = matrix[3][0] * in.x + matrix[3][1] * in.y + matrix[3][2] * in.z + matrix[3][3];

	if (w < 0.001f)
	{
		out.x *= 100000;
		out.y *= 100000;
		return false;
	}

	const auto invw = 1.0f / w;
	out.x *= invw;
	out.y *= invw;
	
	/*printf("%.1f - %.1f - %.1f - %.1f\n%.1f - %.1f - %.1f - %.1f\n%.1f - %.1f - %.1f - %.1f\n%.1f - %.1f - %.1f - %.1f\n\n",
		matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
		matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
		matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
		matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);*/
	scale(out);
	return true;
}
bool c_renderer::world2screen(c_vector3d In, c_vector3d& Out, matrix3x4t matrix)
{
	matrix3x4t ViewMatrix = matrix;

	Out.x = ViewMatrix.matrix[0] * In.x + ViewMatrix.matrix[1] * In.y + ViewMatrix.matrix[2] * In.z + ViewMatrix.matrix[3];
	Out.y = ViewMatrix.matrix[4] * In.x + ViewMatrix.matrix[5] * In.y + ViewMatrix.matrix[6] * In.z + ViewMatrix.matrix[7];
	Out.z = ViewMatrix.matrix[12] * In.x + ViewMatrix.matrix[13] * In.y + ViewMatrix.matrix[14] * In.z + ViewMatrix.matrix[15];
	if (Out.z < 0.01f) return false;
	float Inverse = 1.f / Out.z;
	Out.x *= Inverse;
	Out.y *= Inverse;

	double X = port.Width / 2;
	double Y = port.Height / 2;
	X += 0.5 * Out.x * port.Width + 0.5;
	Y -= 0.5 * Out.y * port.Height + 0.5;
	Out.x = (float)X;
	Out.y = (float)Y;
	if (Out.x > port.Width || Out.x < 0 || Out.y > port.Height || Out.y < 0)return false;
	return true;
}
bool c_renderer::is_on_screen(const c_vector3d& in, const float width, viewmatrix& matrix) const
{
	auto out = c_vector2d(matrix[0][0] * in.x + matrix[0][1] * in.y + matrix[0][2] * in.z + matrix[0][3],
		matrix[1][0] * in.x + matrix[1][1] * in.y + matrix[1][2] * in.z + matrix[1][3]);

	const auto w = matrix[3][0] * in.x + matrix[3][1] * in.y + matrix[3][2] * in.z + matrix[3][3];

	if (w < 0.001f)
	{
		out.x *= 100000;
		out.y *= 100000;
		return false;
	}

	const auto invw = 1.0f / w;
	out.x *= invw;
	out.y *= invw;

	scale(out);
	return !(out.x - width > port.Width || out.x + width < 1.f);
}
