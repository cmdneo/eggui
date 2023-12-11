#include <cassert>

#include "widget.hxx"
#include "theme.hxx"
#include "container.hxx"
#include "graphics.hxx"
#include "canvas.hxx"

using namespace eggui;

void Widget::set_position(Point new_pos)
{
	if (parent)
		abs_position = new_pos + parent->get_absolute_position();
	else
		abs_position = new_pos;

	canvas.set_position(new_pos);
}

Widget *Widget::notify(Event ev)
{
	// Make cursor position relative to the widget.
	ev.cursor -= get_position();
	return notify_impl(ev);
}

void Widget::draw_debug()
{
	if (!is_visible(get_window_size()))
		return;

	const auto pen = acquire_pen();
	draw_debug_impl();
}

void Widget::draw()
{
	if (!is_visible(get_window_size()))
		return;

	const auto pen = acquire_pen();
	clear_background();
	draw_impl();
};

void Widget::draw_debug_impl()
{
	draw_rect_lines(Point(), get_size(), DEBUG_BORDER_COLOR);
}

bool Interactive::handle_mouse_hover_events(Event ev)
{
	if (is_disabled())
		return false;

	switch (ev.type) {
	case EventType::MouseIn:
		is_hovering = true;
		break;
	case EventType::MouseOut:
		is_hovering = false;
		break;
	case EventType::MouseMotion:
		break;

	default:
		return false;
		break;
	}

	return true;
}

bool Interactive::handle_mouse_press_events(Event ev)
{
	if (is_disabled())
		return false;

	switch (ev.type) {
	case EventType::MousePressed:
		is_pressed = true;
		break;
	case EventType::MouseReleased:
		is_pressed = false;
		break;

	default:
		return false;
	}

	return true;
}

Widget *Interactive::notify_impl(Event ev)
{
	if (!is_disabled() && ev.type == EventType::IsInteractive)
		was_any_event_handeled = true;

	if (was_any_event_handeled) {
		was_any_event_handeled = false;
		return this;
	}
	return nullptr;
}

bool Widget::is_visible(Point window_size) const
{
	// Screen rectange points are: (0, 0) and (win_width-1, win_height-1)
	// Check if bounding box of the widget collides with that of the screen.
	return check_box_collision(
		get_absolute_position(), canvas.get_size(), Point(0, 0), window_size
	);
}

Pen Widget::acquire_pen() { return canvas.acquire_pen(); }
