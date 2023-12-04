#ifndef WIDGET_HXX_INCLUDED
#define WIDGET_HXX_INCLUDED

#include <vector>
#include <memory>
#include <utility>

#include "point.hxx"
#include "event.hxx"

namespace eggui
{
class Widget
{
public:
	Widget(int w, int h)
		: Widget(0, 0, w, h)
	{
	}

	Widget(int x, int y, int w, int h)
		: position(x, y)
		, box_size(w, h)
	{
	}

	virtual ~Widget() = default;

	bool is_visible(int win_width, int win_height)
	{
		// Screen rectange points are: (0, 0) and (win_width-1, win_height-1)
		// Check if bounding box of the widget collides with that of the screen.
		return check_box_collision(
			position, box_size, Point(0, 0), Point(win_width, win_height)
		);
	}

	Point get_position() const { return position; }
	Point get_size() const { return box_size; }
	Widget *get_parent() { return parent; }

	/// @brief Set parent of the widget.
	/// @param w Parent widget, it is generally a container
	void set_parent(Widget *w) { parent = w; }

	/// @brief Move the container along with its children
	/// @param new_pos New position
	virtual void set_position(Point new_pos) { position = new_pos; }

	/// @brief Update size re-layout its children(if any) as per its new size.
	/// @param width  New width
	/// @param height New height
	virtual void set_size(int width, int height)
	{
		box_size = Point(width, height);
	}

	/// @brief Handle the event or pass it onto its eligible child(if any).
	/// @param event Event information
	/// @return the widget which handeled the event, nullptr if unhandaled.
	virtual Widget *notify(Event) { return nullptr; }

	/// @brief Calculate the minimum size for the widget
	/// @return Size
	virtual Point calc_min_size() { return get_size(); }

	/// @brief Checks if the `cursor` is inside the widget's boundary.
	/// @param cursor Point
	/// @return True if `cursor` in inside widget, false otherwise.
	virtual bool collides_with_point(Point cursor)
	{
		return cursor.is_in_box(get_position(), get_size());
	}

	/// @brief Draw debug boxes around widget boundaries
	virtual void draw_debug();

	/// @brief Draw the widget
	virtual void draw() = 0;

	// Additional Helper functions
	void set_xpos(int x) { set_position(Point(x, get_position().y)); }
	void set_ypos(int y) { set_position(Point(get_position().x, y)); }

private:
	Widget *parent;
	Point position;
	Point box_size;
};

class Interactive : public Widget
{
public:
	using Widget::Widget;

	Widget *notify(Event ev) override
	{
		if (!is_disabled && ev.type == EventType::IsInteractive)
			return this;
		return nullptr;
	}

	void disable() { is_disabled = true; }
	void enable() { is_disabled = false; }

private:
	bool is_disabled = false;
};
} // namespace eggui

#endif
