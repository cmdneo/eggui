#ifndef WIDGET_HXX_INCLUDED
#define WIDGET_HXX_INCLUDED

#include <vector>
#include <utility>

#include "point.hxx" // Export and use
#include "event.hxx" // Export and use

#include "graphics.hxx"
#include "canvas.hxx"

namespace eggui
{
// Types for widget layout inside a container
//---------------------------------------------------------
enum class Alignment {
	Start,
	Center,
	End,
};

enum class Fill {
	None,
	Row,
	Column,
	RowNColumn,
};

class Widget; // Forward declaration

/// @brief Setup the canvas and draw widget.
/// @param w The widget to be drawn.
void draw_widget(Widget &w);

/// @brief Draw widget debug boxes.
/// @param w The widget to be drawn.
void draw_widget_debug(Widget &w);

/// @brief Do necessary transformations and notify the widget.
/// @param w The widget to be notified.
/// @param ev Event information.
/// @return The widget which responded to the event, nullptr if ignored.
Widget *notify_widget(Widget &w, Event ev);

/// @brief Widget abstract base class, all widget inherit from this.
class Widget
{
	friend void draw_widget(Widget &w);
	friend void draw_widget_debug(Widget &w);
	friend Widget *notify_widget(Widget &, Event ev);

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

	/// @brief Update size re-layout its children(if any) as per its new size.
	/// @param new_size New size
	virtual void set_size(Point new_size);
	inline Point get_size() const { return canvas.get_size(); }

	inline void set_min_size(Point size) { min_box_size = size; }
	inline Point get_min_size() const { return min_box_size; }

	inline void set_max_size(Point size) { max_box_size = size; }
	inline Point get_max_size() const { return max_box_size; }

	// TODO Use preferred size where applicable instead of using minimum size.
	/// @brief Set preferred size, by default preferred size is just min_size.
	/// @param size Preferred size.
	virtual void set_preffered_size(Point size) { set_min_size(size); }
	virtual Point get_preffered_size() const { return get_min_size(); }

	/// @brief Set min, max and current size.
	/// @param size New size
	void set_all_sizes(Point size);

	/// @brief Set parent of the widget.
	/// @param w Parent widget, it is generally a container.
	inline void set_parent(Widget *w) { parent = w; }
	inline Widget *get_parent() const { return parent; }

	/// @brief Move the widget along with its children
	/// @param new_pos New position
	virtual void set_position(Point new_pos);
	inline Point get_position() const { return canvas.get_position(); }

	void set_xpos(int x) { set_position(Point(x, get_position().y)); }
	void set_ypos(int y) { set_position(Point(get_position().x, y)); }

	void set_vert_align(Alignment alignment) { v_align = alignment; }
	Alignment get_vert_align() const { return v_align; }

	void set_horiz_align(Alignment alignment) { h_align = alignment; }
	Alignment get_horiz_align() const { return h_align; }

	void set_fill(Fill fill_mode) { fill = fill_mode; }
	Fill get_fill() const { return fill; }

	/// @brief Checks if the `point` is inside the widget.
	/// @param point Cursor position relative to the widget's parent.
	/// @return Boolean.
	virtual bool collides_with_point(Point point)
	{
		return point.is_in_box(get_position(), get_size());
	}

	/// @brief Calculates the absolute position of the widget on the screen.
	/// @return Point
	Point calc_abs_position() const;

	/// @brief Checks if the widget is visible on the screen if drawn using
	///        the provided pen.
	/// @param pen Pen
	/// @return true if visible, false otherwise.
	bool is_visible(const Pen &pen) const;

protected:
	/// @brief Does the event handling, called by `notify_widget`.
	virtual Widget *notify(Event) { return nullptr; }
	/// @brief Draws stuff, called by `draw_widget`.
	virtual void draw() = 0;
	/// @brief Draws debug boxes, called by `draw_widget_debug`.
	virtual void draw_debug();

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
	// Vertical alignment inside a container
	Alignment v_align = Alignment::Center;
	// Horizontal alignment inside a container
	Alignment h_align = Alignment::Center;
	// Fill mode for excess size inside a container
	Fill fill = Fill::RowNColumn;

	/// Keeps track of whether the drawing will be visible or not,
	/// only valid after we start drawing. For debugging purposes only.
	bool is_drawing_visible = false;
};

/// @brief Abstract base class for interactable widgets.
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
	Widget *notify(Event ev) override;

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

// Types for configuring widget properties
//-------------------------------------------------------------------
enum class Orientation {
	Horizontal = 0,
	Vertical = 1,
};

// Direction can be used as bit flags by extracting its underlying integer.
enum class Direction : std::uint8_t {
	Top = 1,
	Bottom = 1 << 1,
	Left = 1 << 2,
	Right = 1 << 3,
	TopLeft = Top | Left,
	TopRight = Top | Right,
	BottomLeft = Bottom | Left,
	BottomRight = Bottom | Right,
};

/// Ticks per second for update interval(if any) of widgets.
constexpr int TICKS_PER_SECOND = 30;
constexpr double UPDATE_DELTA_TIME = 1.0 / TICKS_PER_SECOND;

// TODO Use a better method for arbitrarily growable widgets.
/// Represents size for an arbtriararily growable widget.
// Using a large enough value but not the maximum ensures no overflow when
// doing arithmetic with it, and a large enough size for all screens.
constexpr int UNLIMITED_MAX_SIZE = 1e6;
} // namespace eggui

#endif
