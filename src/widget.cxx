#include "raylib/raylib.h"

#include "theme.hxx"
#include "event.hxx"
#include "widget.hxx"

using namespace eggui;

void Widget::draw_debug()
{
	DrawRectangleLines(
		position.x, position.y, box_size.x, box_size.y, DEBUG_BORDER_COLOR
	);
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