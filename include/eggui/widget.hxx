#ifndef WIDGET_HXX_INCLUDED
#define WIDGET_HXX_INCLUDED

#include <climits>
#include <vector>
#include <memory>
#include <utility>

#include "point.hxx" // Re-export and use
#include "event.hxx" // Re-export and use

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
		, min_box_size(w, h)
		, max_box_size(w, h)
	{
	}

	virtual ~Widget() = default;

	inline Point get_position() const { return canvas.get_position(); }
	inline Point get_size() const { return canvas.get_size(); }
	inline Point get_min_size() const { return min_box_size; }
	inline Point get_max_size() const { return max_box_size; }
	inline Widget *get_parent() const { return parent; }

	/// @brief Update size re-layout its children(if any) as per its new size.
	/// @param new_size  New size
	virtual void set_size(Point new_size);

	void set_min_size(Point size) { min_box_size = size; }
	void set_max_size(Point size) { max_box_size = size; }

	/// @brief Set parent of the widget.
	/// @param w Parent widget, it is generally a container.
	void set_parent(Widget *w) { parent = w; }

	/// @brief Move the widget along with its children
	/// @param new_pos New position
	virtual void set_position(Point new_pos);

	void set_xpos(int x) { set_position(Point(x, get_position().y)); }
	void set_ypos(int y) { set_position(Point(get_position().x, y)); }

	/// @brief Checks if the `cursor` is inside the widget's boundary.
	/// @param cursor Point
	/// @return True if `cursor` in inside widget, false otherwise.
	virtual bool collides_with_point(Point cursor)
	{
		return cursor.is_in_box(get_position(), get_size());
	}

	/// @brief Calculates the absolute position of the widgte on the screen.
	/// @return Point
	Point calc_abs_position() const;

	/// @brief Checks if the widget is visible on the screen if drawn using
	///        the provided pen.
	/// @param pen Pen
	/// @return true if visible, false otherwise.
	bool is_visible(const Pen &pen) const;

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
	/// Parent widget, at a time a widget can have only one parent.
	Widget *parent = nullptr;
	// Position relative to the parent and size of the widget are the same as
	// that of the canvas, so we do not need to store them again.
	/// Canvas on which the widget will be drawn.
	Canvas canvas;
	/// Minimum widget size
	Point min_box_size;
	/// Maximum widget size
	Point max_box_size;

	/// Keeps track of whether the drawing will be visible or not,
	/// only valid after we start drawing. For debugging purposes only.
	bool is_drawing_visible = false;
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
};

/// Represents an arbtriararily growable widget.
// Using INT_MAX/2 ensures no overflow and a large enough size.
constexpr Point UNLIMITED_MAX_SIZE(INT_MAX / 2, INT_MAX / 2);

} // namespace eggui

#endif
