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
	/// @brief Create blank canvas, no texture is allocated.
	Canvas() = default;
	Canvas(const Canvas &) = delete;

	Canvas(Canvas &&c)
		: texture_id(c.texture_id)
		, size(c.size)
		, position(c.position)
		, region_start(c.region_start)
		, region_size(c.region_size)
	{
	}

	/// @brief Create a canvas with texture size, no texture is allocated
	/// if any component of the size is zero.
	/// @param position Position relative to the screen/parent-canvas.
	/// @param size Texture size.
	Canvas(Point position_, Point size_);

	~Canvas();

	/// @brief Acquire a pen for the canvas.
	/// @return Pen
	/// @note Only one pen can be active per canvas.
	Pen acquire_pen();
	bool has_active_pen() { return active_pen_cnt != 0; }

	/// @brief Set region of the texture which should be drawn.
	/// @param rect_start Region rectangle start.
	/// @param rect_size Region rectangle size.
	void set_draw_region(Point rect_start, Point rect_size);

	/// @brief Resize the canvas along with underlying texture. It's Expensive.
	/// It also resets the draw region to default.
	/// @param new_size New size for the canvas and the texture.
	void resize_texture(Point new_size);

	void set_position(Point new_pos) { position = new_pos; }
	Point get_position() const { return position; }
	Point get_size() const { return size; }

private:
	// There can be only one active pen per canvas.
	int active_pen_cnt = 0;
	/// Texture ID for the widget. -1 means no texture allocated.
	int texture_id = -1;
	// Full texture size. It is equal to the widget size.
	Point size;
	// Position on where the texture will be drawn.
	Point position;

	// Region of the texture which should be drawn.
	// By default full texture is drawn.
	Point region_start;
	Point region_size;
};

/// @brief Draw to the canvas from which it was acquired.
/// @details
/// We need to acquire a pen for canvas to start to start drawing on it.
/// This because, before starting the drawing we need to push the texture for
/// and after the drawing finishes we need to pop-off the texture and draw it
/// on the screen/parent-canvas.
/// If a widget has parent canvas then we need to draw that widget on
/// Therefore, we use this class for doing those steps. As soon as the pen goes
/// out of the scope the texture is popped and drawn to screen/parent-canvas.
///
/// If multiple pens are active then, the most recently acquired pen will draw
/// on the canvas of the pen acquired just before it.
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
};

} // namespace eggui
#endif