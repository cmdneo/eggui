#include "window.hxx"
#include "input.hxx"

#include "raylib/raylib.h"

using namespace eggui;

TextInput::TextInput(int w, int h)
	: Interactive(w, h)
	, text(w, h, "abc123")
{
}

Widget *TextInput::notify_impl(Event ev)
{
	auto blink = [this](int tick) {
		switch (tick) {
		case 0:
			text.set_cursor(true);
			break;
		case 15:
			text.set_cursor(false);
			break;

		default:
			return false;
		}

		return true;
	};

	// TODO Handle more possible keypresses and text select.
	switch (ev.type) {
	case EventType::MouseIn:
		set_cursor_shape(CursorShape::IBeam);
		// ev.window.request_animation(this);
		return this;

	case EventType::MouseOut:
		set_cursor_shape(CursorShape::Default);
		// ev.window.stop_animation(this);
		return this;

	case EventType::MousePressed:
		text.notify(ev);
		return this;

	case EventType::CharEntered:
		text.insert_before_cursor(ev.keycode);
		return this;

	case EventType::KeyPressed:
		if (ev.keycode == KEY_BACKSPACE)
			text.delete_before_cursor();
		if (ev.keycode == KEY_LEFT)
			text.move_cursor(-1);
		if (ev.keycode == KEY_RIGHT)
			text.move_cursor(1);
		return this;

	default:
		break;
	}

	return Interactive::notify_impl(ev);
}

void TextInput::draw_impl() { text.draw(); }