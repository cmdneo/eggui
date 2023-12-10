#include <cassert>

#include "raylib/raylib.h"

#include "theme.hxx"
#include "event.hxx"
#include "widget.hxx"
#include "container.hxx"
#include "graphics.hxx"
#include "canvas.hxx"

using namespace eggui;

Pen Widget::acquire_pen() { return canvas.acquire_pen(); }

Widget *Widget::notify(Event ev)
{
	// Make cursor position relative to the widget.
	ev.cursor -= get_position();
	return notify_impl(ev);
}

void Widget::draw_debug()
{
	const auto pen = acquire_pen();
	draw_debug_impl();
}

void Widget::draw()
{
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
