#ifndef MANAGERS_HXX_INCLUDED
#define MANAGERS_HXX_INCLUDED

#include <vector>
#include <utility>

#include "raylib/raylib.h"
#include "point.hxx"

class ClippingManager
{
	using Point = eggui::Point;

public:
	static ClippingManager &instance();

	/// @brief Pushes a new clip area, the resulting clip area is the
	/// intersection of all the previous clip areas pushed.
	/// @param start Area start position on screen.
	/// @param size  Area size.
	void push_clip_area(Point start, Point size);
	/// @brief Restores the last clip area.
	void pop_clip_area();

	/// @brief Get current clip area on screen.
	/// @return Clip area start and size.
	/// @note If nothing is being clipped then returns screen area.
	std::pair<Point, Point> get_current_clip_area();

	/// @brief Just calculate the resulting clipping area, do not apply it.
	/// @param start Area start position on screen.
	/// @param size  Area size.
	/// @return Clip area start and size.
	std::pair<Point, Point> calc_clip_area(Point start, Point size);

private:
	ClippingManager() = default;

	/// Stores the clip area for each push operation for the purpose of
	/// restoring them when `pop_clip_area` is called.
	std::vector<std::pair<Point, Point>> clip_areas;
};

// TODO complete this
class FontManager
{
public:
	static FontManager &instance();

	int load_font(
		const char *file_type, unsigned char *data, int size, int font_size
	);
	void unload_fonts();

private:
	FontManager() { fonts.reserve(8); }

	std::vector<Font> fonts;
};

#endif