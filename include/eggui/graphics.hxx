/// A wrapper around graphics library we are using.
/// This supports drawing primitive shapes and images.

#ifndef GRAPHICS_HXX_INCLUDED
#define GRAPHICS_HXX_INCLUDED

#include <cstdint>

#include "point.hxx"

namespace eggui
{
enum class FontSize {
	Tiny,
	Small,
	Medium,
	Large,
	Huge,
};

constexpr int FONT_SIZE_COUNT = 5;

struct RGBA {
	constexpr RGBA(
		std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_ = 255
	)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_)
	{
	}

	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
	std::uint8_t a = 255;
};

// TODO Use font size as per size of the display
// Font height in units, where 1 unit = 0.28 mm.
// Indexed by FontSize
// constexpr int FONT_HEIGHTS[FONT_SIZE_COUNT] = {14, 18, 24, 32, 42};
// constexpr double MM_PER_UNIT = 0.28;

/// @brief Must be called to load the fonts and do other setup work.
///
/// @note Implementation: It should be called after the window has been
/// initialized and OpenGL context is available.
void init_graphics();
/// @brief Call before exiting the application.
void deinit_graphics();

/// @brief Apply a translation to the drawing position.
/// @param pos Amount to translate.
void push_translation(Point pos);
/// @brief Removes the last applied translation.
void pop_translation();
/// @brief Calculates total translation till now.
/// @return Accumulated translation position.
Point get_total_translation();

/// @brief Get window size in pixels
/// @return Point: width and height
Point get_window_size();

// Basic drawing functions
// They draw according to the translation applied using
// push/pop_translation.
//--------------------------------------------------------------
// clang-format off
void clear_background();
void draw_pixel(Point v, RGBA color);
void draw_line(Point start, Point end, RGBA color);
void draw_rect(Point position, Point size, RGBA color);
void draw_rect_lines(Point position, Point size, RGBA color);
void draw_rounded_rect(Point position, Point size, float round, RGBA color);
void draw_cirlce(Point center, float radius, RGBA color);
void draw_cirlce_sector(Point position, float radius, float start_angle, float end_angle, RGBA color);
void draw_ring(Point center, float inner_rad, float outer_rad, RGBA color);
void draw_triangle(Point v1, Point v2, Point v3, RGBA color);
void draw_text(Point position, RGBA color, const char *text, FontSize font_size);

Point tell_text_size(const char *text, FontSize font_size);
} // namespace eggui

#endif
