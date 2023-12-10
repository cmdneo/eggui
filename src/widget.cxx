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

void Widget::draw_debug()
{
	// This function is called by its subclasses to draw the boundary
	// so check then acquire a pen.
	if (!canvas.has_active_pen()) {
		const auto pen = acquire_pen();
		draw_rect_lines(Point(), get_size(), DEBUG_BORDER_COLOR);
	} else {
		draw_rect_lines(Point(), get_size(), DEBUG_BORDER_COLOR);
	}
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

	default:
		return false;
		break;
	}

	return true;
}