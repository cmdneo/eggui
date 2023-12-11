#ifndef WIDGET_HXX_INCLUDED
#define WIDGET_HXX_INCLUDED

#include <vector>
#include <memory>
#include <utility>

// Point and Event are re-exported by widget.hxx, so need to include these if
// widget.hxx is included by the source file.
// TODO impl above in all.
#include "point.hxx"
#include "event.hxx"
#include "graphics.hxx"
#include "canvas.hxx"

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
		: canvas(Point(x, y), Point(w, h))
	{
	}

	virtual ~Widget() = default;

	bool is_visible(int win_width, int win_height)
	{
		// Screen rectange points are: (0, 0) and (win_width-1, win_height-1)
		// Check if bounding box of the widget collides with that of the screen.
		return check_box_collision(
			canvas.get_position(), canvas.get_size(), Point(0, 0),
			Point(win_width, win_height)
		);
	}

	Point get_position() const { return canvas.get_position(); }
	Point get_size() const { return canvas.get_size(); }
	Widget *get_parent() { return parent; }

	/// @brief Set parent of the widget.
	/// @param w Parent widget, it is generally a container.
	void set_parent(Widget *w) { parent = w; }

	/// @brief Move the widget along with its children
	/// @param new_pos New position
	virtual void set_position(Point new_pos) { canvas.set_position(new_pos); }

	/// @brief Update size re-layout its children(if any) as per its new size.
	/// @param width  New width
	/// @param height New height
	virtual void set_size(Point new_size) { canvas.set_size(new_size); }

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

	// We wrap all the drawing and notification methods to setup
	// the pen and adjust mouse position relative to the widget.
	// Actual implementations are in the `*_impl` virtual method.
	//--------------------------------------------------------------------

	/// @brief Handle the event or pass it onto its eligible child(if any).
	/// @param event Event information
	/// @return the widget which handeled the event, nullptr if unhandaled.
	/// @note For overriding override the `notify_impl` method.
	Widget *notify(Event ev);
	/// @brief Draw debug boxes around widget boundaries
	/// @note For overriding override the `debug_draw_impl` method.
	void draw_debug();
	/// @brief Draw the widget
	/// @note For overriding override the `draw_impl` method.
	void draw();

	// Additional Helper functions
	void set_xpos(int x) { set_position(Point(x, get_position().y)); }
	void set_ypos(int y) { set_position(Point(get_position().x, y)); }

	/// @brief Transforms event cursor to be relative to the current widget,
	/// that is the cursor will be relative to the parent, just like position.
	/// @param ev Event
	/// @return Event with transformed cursor
	Event pass_event(Event ev)
	{
		ev.cursor -= get_position();
		return ev;
	}

protected:
	/// @brief Does the event handling, called by `notify`.
	virtual Widget *notify_impl(Event) { return nullptr; }
	/// @brief Draws stuff, called by `draw`.
	virtual void draw_impl() = 0;
	/// @brief Draws debug boxes, called by `draw_debug`.
	virtual void draw_debug_impl();

private:
	/// @brief Acquire the pen of the canvas for drawing the widget.
	/// @return Pen
	Pen acquire_pen();

	// Parent widget, at a time a widget can have only one parent.
	Widget *parent = nullptr;
	// Position relative to the parent and size of the widget are the same as
	// that of the canvas, so we do not need to store them again.
	/// Canvas on which the widget will be drawn.
	Canvas canvas;
};

class Interactive : public Widget
{
public:
	using Widget::Widget;

	void disable() { is_disabled_ = true; }
	void enable() { is_disabled_ = false; }
	bool is_disabled() const { return is_disabled_; }

protected:
	/// @brief Call and return its value for all the events that are ignored
	///        by the subclass method which overrides this.
	/// @param ev Event
	/// @return The widget which handeled the event.
	Widget *notify_impl(Event ev) override;

	/// @brief Manage is-hovering state for an enabled interactive widget.
	/// @param ev Event
	/// @return true if any of the events related to it were handled.
	bool handle_mouse_hover_events(Event ev);

	/// @brief Manage is-pressed state for an enabled interactive widget.
	/// @param ev Event
	/// @return true if any of the events related to it were handled.
	bool handle_mouse_press_events(Event ev);

	// State information for interactive widgets
	bool is_hovering = false;
	bool is_pressed = false;

private:
	bool is_disabled_ = false;
	bool was_any_event_handeled = false;
};
} // namespace eggui

#endif
