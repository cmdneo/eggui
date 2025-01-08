#include "draggable.hxx"

using namespace eggui;

auto Draggable::notify(Event ev) -> Widget *
{
	switch (ev.type) {
	case EventType::MousePressed:
		set_cursor_shape(CursorShape::ResizeOmni);
		break;
	case EventType::MouseReleased:
		set_cursor_shape(CursorShape::Default);
		break;

	case EventType::MouseDrag:
		on_drag(ev.window, *this, ev.delta);
		break;

	default:
		return nullptr;
	}

	return nullptr;
}

void Draggable::draw() { /* Nothing for now */ }