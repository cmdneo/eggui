#ifndef MANAGERS_HXX_INCLUDED
#define MANAGERS_HXX_INCLUDED

#include <vector>
#include <utility>

#include "raylib/raylib.h"
#include "point.hxx"

namespace eggui
{

/// @brief Nested draw region clipping.
class ClippingManager
{
public:
	static ClippingManager &instance();

	/// @brief Pushes a new clip area, the resulting clip area is the
	/// intersection of all the previous clip areas pushed.
	/// @param start Area start position on screen.
	/// @param size  Area size.
	void push_clip_area(Point start, Point size);
	/// @brief Restores the last clip area.
	void pop_clip_area();

	/// @brief Temporarily disable clipping.
	/// @note While clipping is disabled push/pop should not be used.
	void disable();
	/// @brief Re-enable clipping and resotre the last clip region.
	void enable();

	/// @brief Get current clip area on screen.
	/// @return Clip area start and size.
	/// @note If nothing is being clipped then returns screen area.
	std::pair<Point, Point> get_current_clip_region() const;

	/// @brief Just calculate the resulting clipping area, do not apply it.
	/// @param start Area start position on screen.
	/// @param size  Area size.
	/// @return Clip area start and size.
	std::pair<Point, Point> calc_clip_area(Point start, Point size) const;

private:
	ClippingManager() = default;

	/// Stores the clip area for each push operation for the purpose of
	/// restoring them when `pop_clip_area` is called.
	std::vector<std::pair<Point, Point>> clip_areas;
	bool is_enabled = true;
};

// class TranslationManager
// {
// public:
// 	static TranslationManager &instance();

// 	void push_translation(Point pos);
// 	void pop_translation(Point pos);

// 	Point get_total_translation() const;

// private:
// 	TranslationManager() = default;
// };

// TODO complete font manager
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

} // namespace eggui

#endif