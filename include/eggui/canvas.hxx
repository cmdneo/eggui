#ifndef CANVAS_HXX_INCLUDED
#define CANVAS_HXX_INCLUDED

#include <utility>

#include "point.hxx"
#include "graphics.hxx"

namespace eggui
{
class Pen; // Forward declaration

/// @brief We draw every widget on a canvas to support scrollable views,
/// clipping and relative coordinates.
///
/// @details
/// Before drawing using draw_* functions, a pen needs to be acquired from the
/// canvas so that things get drawn at the right position on the canvas.
/// The acquired pen should stay in the scope until the drawing is complete.
///
/// @example
/// @code {.cpp}
/// 	const Pen pen = canvas.acquire_pen();
/// 	draw_circle(Point(50, 50), 21, RGBA(255, 0, 0));
/// @endcode
///
/// @warning Do not move the canvas while it has an active pen.
class Canvas
{
	friend class Pen;

public:
	Canvas() = default;

	Canvas(Canvas &&c) = default;
	Canvas(const Canvas &) = delete;

	/// @brief Create a canvas.
	/// @param position Position relative to the screen/parent-canvas.
	/// @param size Canvas size.
	Canvas(Point position_, Point size_)
		: size(size_)
		, position(position_)
		, region_start(Point(0, 0))
		, region_size(size_)
	{
	}

	/// @brief Acquire a pen for the canvas.
	/// @param enable_clipping Enables draw region clipping.
	/// @return Pen
	/// @note Only one pen can be active per canvas.
	Pen acquire_pen(bool enable_clipping = true);
	/// @brief Check if a pen is active for the canvas.
	/// @return boolean
	bool has_active_pen() const { return active_pen_cnt != 0; }

	/// @brief Set region to which drawing should be restricted to.
	/// @param rect_start Region rectangle start.
	/// @param rect_size Region rectangle size.
	void set_draw_region(Point rect_start, Point rect_size);

	/// @brief Resize the canvas and reset the draw region to default.
	/// @param new_size New size for the canvas.
	void set_size(Point new_size);

	/// @brief Set postion of the canvas
	/// @param new_pos New position.
	void set_position(Point new_pos) { position = new_pos; }

	/// @brief Get region of the canvas which will be drawn on the screen.
	/// @return Pair represents a rectangle as position and size.
	std::pair<Point, Point> get_draw_region()
	{
		return {region_start, region_size};
	}

	inline Point get_size() const { return size; }
	inline Point get_position() const { return position; }

private:
	// There can be only one active pen per canvas.
	int active_pen_cnt = 0; // Managed by Pen class

	// Canvas size.
	Point size{};
	// Canvas position.
	Point position{};

	// Region of the canvas which should be drawn.
	// By default full canvas is drawn.
	Point region_start{};
	Point region_size{};
};

/// @brief Draw to the canvas from which it was acquired.
///
/// @details
/// We need to acquire a pen for canvas to start to start drawing on it.
/// This is because, before starting the drawing we need to apply the
/// translation and after the drawing finishes we need to remove the
/// translation applied on the canvas.
///
/// If multiple pens are active then(of different canvases), then the most
/// recently acquired pen will be used to draw.
/// This is the key to drawing using relative positions.
class Pen
{
public:
	Pen(Canvas &canvas_, bool is_clipping_);
	~Pen();

	Pen(Pen &&pen) = default;
	Pen(const Pen &) = delete;

	/// @brief Gets clip region in which the pen can actually draw.
	/// @return Rectangle: relative(to canvas) position and size.
	std::pair<Point, Point> get_clip_region() const
	{
		return {clip_rect_pos, clip_rect_size};
	}

	/// @brief Does absolutely nothing, its only purpose is to tell the
	/// compiler that the pen is not an unused variable. Use if needed.
	void use() {}

private:
	Canvas &canvas;
	bool is_clipping = true;

	// Actual clip region produced by the intersection of previously pushed
	// clip regions, calculated even if the pen is not clipping.
	// Position is relative to the canvas it refers to.
	Point clip_rect_pos{};
	Point clip_rect_size{};
};

} // namespace eggui
#endif