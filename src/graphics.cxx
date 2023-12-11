#include <cassert>
#include <algorithm>
#include <vector>
#include <memory>
#include <ranges>
#include <utility>

#include "raylib/raylib.h"
#include "raylib/rlgl.h"

#include "roboto_mono.bin.h"
#include "graphics.hxx"
#include "point.hxx"
#include "theme.hxx"
#include "canvas.hxx"

using namespace eggui;

/// Font size in pixels for FontSize variants, indexed by FontSize.
constexpr int FONT_PX_SIZES[FONT_SIZE_COUNT] = {18, 24, 32, 42, 56};

/// Font glyphs need to be rendered for each font size, indexed by FontSize.
/// Since we load only a fixed number of fonts, we dont really need manager for it.
static Font g_mono_fonts[FONT_SIZE_COUNT];

// Conversion and functions
//---------------------------------------------------------
constexpr inline Color COLOR(RGBA rgba)
{
	return Color{rgba.r, rgba.g, rgba.b, rgba.a};
}

constexpr inline Vector2 VEC2(Point point)
{
	return Vector2{static_cast<float>(point.x), static_cast<float>(point.y)};
}

inline Rectangle points_to_rect(Point pos, Point size)
{
	Rectangle ret;
	ret.x = pos.x;
	ret.y = pos.y;
	ret.width = size.x;
	ret.height = size.y;

	return ret;
}

// Calculation & transformation functions
//---------------------------------------------------------
inline Rectangle shrink_from_center(Rectangle rect, float x, float y)
{
	rect.x += x;
	rect.y += y;
	rect.height -= 2 * x;
	rect.width -= 2 * y;
	return rect;
}

inline int calc_segments(float radius, float angle = 360.0)
{
	angle = std::abs(angle);
	// Too many segments value causes artifacts at corners when AA4X is enabled.
	return radius * 1.5 * angle / 360.;
}

inline int fsize_to_index(eggui::FontSize font_size)
{
	return static_cast<int>(font_size);
}

namespace eggui
{

void init_graphics()
{
	for (int i = 0; i < FONT_SIZE_COUNT; ++i) {
		g_mono_fonts[i] = LoadFontFromMemory(
			".ttf", ROBOTO_MONO_TTF, ROBOTO_MONO_TTF_LEN, FONT_PX_SIZES[i],
			nullptr, 0
		);
	}
}

void deinit_graphics()
{
	for (auto &font : g_mono_fonts)
		UnloadFont(font);
}

void push_translation(Point pt)
{
	rlPushMatrix();
	rlTranslatef(pt.x, pt.y, 0);
}

void pop_translation() { rlPopMatrix(); }

Point get_total_translation()
{
	auto tmat = rlGetMatrixTransform();
	return Point(tmat.m12, tmat.m13);
}

Point get_window_size() { return Point(GetScreenWidth(), GetScreenHeight()); }

void clear_background() { ClearBackground(COLOR(BACKGROUND_COLOR)); }

void draw_pixel(Point v, RGBA color) { DrawPixel(v.x, v.y, COLOR(color)); }

void draw_line(Point start, Point end, RGBA color)
{
	DrawLine(start.x, start.y, end.x, end.y, COLOR(color));
}

void draw_rect(Point position, Point size, RGBA color)
{
	DrawRectangle(position.x, position.y, size.x, size.y, COLOR(color));
}

void draw_rect_lines(Point position, Point size, RGBA color)
{
	DrawRectangleLines(position.x, position.y, size.x, size.y, COLOR(color));
}

void draw_rounded_rect(Point position, Point size, float round, RGBA color)
{
	auto rect = points_to_rect(position, size);
	auto segs = calc_segments(std::min(position.x, position.y) / 2. * round);
	DrawRectangleRounded(rect, round, segs, COLOR(color));
}

void draw_cirlce(Point center, float radius, RGBA color)
{
	DrawCircle(center.x, center.y, radius, COLOR(color));
}

void draw_cirlce_sector(
	Point position, float radius, float start_angle, float end_angle, RGBA color
)
{
	int segs = calc_segments(radius, end_angle - start_angle);
	DrawCircleSector(
		VEC2(position), radius, start_angle, end_angle, segs, COLOR(color)
	);
}
void draw_ring(Point center, float inner_rad, float outer_rad, RGBA color)
{
	DrawRing(
		VEC2(center), inner_rad, outer_rad, 0, 360., calc_segments(outer_rad),
		COLOR(color)
	);
}

void draw_triangle(Point v1, Point v2, Point v3, RGBA color)
{
	DrawTriangle(VEC2(v1), VEC2(v2), VEC2(v3), COLOR(color));
}

void draw_text(Point position, RGBA color, const char *text, FontSize font_size)
{

	int idx = fsize_to_index(font_size);
	DrawTextEx(
		g_mono_fonts[idx], text, VEC2(position), FONT_PX_SIZES[idx], 0,
		COLOR(color)
	);
}

Point tell_text_size(const char *text, FontSize font_size)
{
	int idx = fsize_to_index(font_size);
	auto sz = MeasureTextEx(g_mono_fonts[idx], text, FONT_PX_SIZES[idx], 0);
	return Point(sz.x, sz.y);
}
} // namespace eggui
