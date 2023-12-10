#include <algorithm>

#include "graphics.hxx"
#include "canvas.hxx"
#include "button.hxx"
#include "theme.hxx"

using namespace eggui;

Widget *Button::notify_impl(Event ev)
{
	if (handle_mouse_hover_events(ev))
		return this;
	if (handle_mouse_press_events(ev))
		return this;

	if (ev.type == EventType::MouseClick) {
		on_click(*this);
		return this;
	}

	return Interactive::notify_impl(ev);
};

void Button::draw_impl()
{
	auto color = BUTTON_COLOR;
	if (is_pressed)
		color = BUTTON_CLICK_COLOR;
	else if (is_hovering)
		color = BUTTON_HOVER_COLOR;

	draw_rounded_rect(Point(), get_size(), 0.25, color);

	// Draw centered text
	auto text_size = tell_text_size(label, FontSize::Small);
	auto text_pos = (get_size() - text_size) / 2;
	draw_text(text_pos, RGBA(255, 255, 255), label, FontSize::Small);
}