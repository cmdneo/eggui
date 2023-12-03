#ifndef WIDGET_HXX_INCLUDED
#define WIDGET_HXX_INCLUDED

#include <vector>
#include <memory>
#include <utility>
#include <typeinfo>

#include "point.hxx"
#include "event.hxx"

namespace eggui
{
enum class Anchor {
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

class Widget
{
public:
	Anchor anchor_corner;

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
		return position.x + box_size.x >= 0 && position.y + box_size.y >= 0
			   && position.x < win_width && position.y < win_height;
	}

	Point get_position() const { return position; }
	Point get_size() const { return box_size; }

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

	/// @brief Notify of an event on it.
	/// @param event
	/// @return Returns false if the event is ignored.
	virtual bool notify(Event) { return false; }

	/// @brief Calculate the minimum size for the widget
	/// @return Size
	virtual Point calc_min_size() { return get_size(); }

	/// @brief Draw debug boxes around widget boundaries
	virtual void draw_debug();

	/// @brief Draw the widget
	virtual void draw() = 0;

	/// @brief Returns the first active widget(itself included) it contains
	///        whose bounding box collides with `point`.
	/// @param point Point for position
	/// @return nullptr if no collision with any active widget it.
	///
	/// @note Active widget is a widget to which may respond to events.
	virtual Widget *get_active_widget_at(Point point) = 0;

	// Additional Helper functions
	void set_xpos(int x) { set_position(Point(x, get_position().y)); }
	void set_ypos(int y) { set_position(Point(get_position().x, y)); }

private:
	Point position;
	Point box_size;
};

class InteractiveWidget : public Widget
{
public:
	using Widget::Widget;

	Widget *get_active_widget_at(Point point) override
	{
		if (is_disabled)
			return nullptr;
		return point.is_in_box(get_position(), get_size()) ? this : nullptr;
	}

private:
	bool is_disabled = false;
};
} // namespace eggui

#endif
