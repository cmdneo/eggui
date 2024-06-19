#include <cmath>
#include <algorithm>

#include "window.hxx"
#include "input.hxx"
#include "theme.hxx"

#include "raylib/raylib.h"

using namespace eggui;

// TODO Implement auto height calculation
TextInput::TextInput(int w, int h)
	: Interactive(w, h)
	, text(w - 2 * TEXT_PADDING, get_size().y - 2 * TEXT_PADDING)
{
	text.set_position(Point(TEXT_PADDING, TEXT_PADDING));
	text.set_cursor_opacity(false);
}
void TextInput::set_size(Point size)
{
	text.set_all_sizes(size - 2 * Point(TEXT_PADDING, TEXT_PADDING));
	Widget::set_size(size);
}

Widget *TextInput::notify_impl(Event ev)
{
	auto blink = [this](int, float progress) {
		// Use 1 - (2x - 1)^2 for the blink animation of the cursor.
		double opacity = 1 - std::pow(2. * progress - 1, 2);
		text.set_cursor_opacity(opacity);

		return true;
	};

	// TODO Handle more possible keypresses and text select.
	switch (ev.type) {
	case EventType::FocusGained:
		ev.window.request_animation(
			this, Animation(TICKS_PER_SECOND, 0, true, blink)
		);
		return this;

	case EventType::FocusLost:
		text.set_cursor_opacity(0);
		ev.window.request_remove_animations(this);
		return this;

	case EventType::MouseIn:
		set_cursor_shape(CursorShape::IBeam);
		return this;

	case EventType::MouseOut:
		set_cursor_shape(CursorShape::Default);
		return this;

	case EventType::MousePressed:
		ev.window.request_focus(this, true);
		return text.notify(ev);

	case EventType::CharEntered:
		text.insert_before_cursor(ev.keycode);
		return this;

	case EventType::KeyPressed:
		if (ev.keycode == KEY_DELETE)
			text.delete_after_cursor();
		else if (ev.keycode == KEY_BACKSPACE)
			text.delete_before_cursor();
		else if (ev.keycode == KEY_LEFT)
			text.move_cursor(-1);
		else if (ev.keycode == KEY_RIGHT)
			text.move_cursor(1);

		return this;

	default:
		break;
	}

	return Interactive::notify_impl(ev);
}

void TextInput::draw_impl()
{
	draw_rounded_rect(Point(), get_size(), ELEMENT_ROUNDNESS, TEXT_BG_COLOR);
	text.draw();
}

void TextInput::draw_debug_impl()
{
	text.draw_debug();
	Interactive::draw_debug_impl();
}