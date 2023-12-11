#ifndef CANVAS_HXX_INCLUDED
#define CANVAS_HXX_INCLUDED

#include "point.hxx"
#include "graphics.hxx"

namespace eggui
{
class Pen; // Forward declaration

/// @brief We draw a every widget on a canvas to support scrolling views and
/// and drawing a widget only when it changes.
///
/// Before starting drawing using draw_* functions, a pen needs to be acquired
/// from the canvas and that pen should stay in the scope until the drawing is
/// complete.
///
/// @code {.cpp}
/// 	Pen pen = canvas.acquire_pen(&parent_canvas);
/// 	pen.use();
/// 	draw_circle(Point(50, 50), 21, RGBA{255, 0, 0, 255});
/// @endcode
class Canvas
{
	friend class Pen;

public:
	Canvas() = default;
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

	Canvas(Canvas &&c)
		: size(c.size)
		, position(c.position)
		, region_start(c.region_start)
		, region_size(c.region_size)
	{
	}

	/// @brief Acquire a pen for the canvas.
	/// @return Pen
	/// @note Only one pen can be active per canvas.
	Pen acquire_pen();
	bool has_active_pen() { return active_pen_cnt != 0; }

	/// @brief Set region to which drawing should be restricted to.
	/// @param rect_start Region rectangle start.
	/// @param rect_size Region rectangle size.
	void set_draw_region(Point rect_start, Point rect_size);

	/// @brief Resize the canvas and reset the draw region to default.
	/// @param new_size New size for the canvas.
	void set_size(Point new_size);

	void set_position(Point new_pos) { position = new_pos; }
	Point get_position() const { return position; }
	Point get_size() const { return size; }

private:
	// There can be only one active pen per canvas.
	int active_pen_cnt = 0;
	// It is equal to the widget size.
	Point size;
	// Position on where the canvas will be drawn.
	Point position;

	// Region of the canvas which should be drawn.
	// By default full canvas is drawn.
	Point region_start;
	Point region_size;
};

/// @brief Draw to the canvas from which it was acquired.
/// @details
/// We need to acquire a pen for canvas to start to start drawing on it.
/// This is because, before starting the drawing we need to apply the
/// translation and after the drawing finishes we need to remove the
/// translation applied on the canvas.
///
/// If multiple pens are active then, the most recently acquired pen will be
/// used to draw on its corresponding canvas.
/// This is the key to drawing a to the parent widget's canvas.
class Pen
{
public:
	Pen(Canvas &canvas_);
	~Pen();

	Pen(const Pen &) = delete;
	Pen(Pen &&pen)
		: canvas(pen.canvas)
	{
	}

	/// @brief Does absolutely nothing, its only purpose is to tell the
	/// compiler that the pen is not an unused variable. Use if needed.
	void use() {}

private:
	Canvas &canvas;
	/// Keeps track of the offset we are at.
	Point offset;
};

} // namespace eggui
#endif