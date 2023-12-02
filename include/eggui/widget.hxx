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

class Window; // Forward declaration for friending

class Widget
{
	friend class Window;

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

	Point get_position() const { return position; }
	Point get_size() const { return box_size; }

	bool is_visible(int win_width, int win_height)
	{
		// Screen rectange points are: (0, 0) and (win_width-1, win_height-1)
		// Check if bounding box of the widget collides with that of the screen.
		return position.x + box_size.x >= 0 && position.y + box_size.y >= 0
			   && position.x < win_width && position.y < win_height;
	}

	/// @brief Move the container along with its children
	/// @param new_pos New position
	void set_position(Point new_pos) { move_position(new_pos - position); }

	/// @brief Notify of an event on it.
	/// @param event
	/// @return Returns false if the event is ignored.
	virtual bool notify(Event) { return false; }

	virtual void draw_debug();

	virtual void draw() = 0;

protected:
	Point position;
	Point box_size;

	std::vector<std::unique_ptr<Widget>> children;

private:
	void move_position(Point pos_delta)
	{
		position += pos_delta;

		for (auto &c : children)
			c->move_position(pos_delta);
	}
};

class InteractiveWidget : public Widget
{
public:
	using Widget::Widget;
};
} // namespace eggui

#endif
