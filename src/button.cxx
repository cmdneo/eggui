#include <algorithm>
#include <string>

#include "graphics.hxx"
#include "canvas.hxx"
#include "button.hxx"
#include "theme.hxx"

using namespace eggui;

Button::Button(int w, int h, std::string txt)
	: Interactive(w, h)
	, label(w, h, txt, RGBA(255, 255, 255))
{
	label.set_parent(this);
	label.set_text_align(Alignment::Center, Alignment::Center);
}

void Button::set_size(Point new_size)
{
	Interactive::set_size(new_size);
	label.set_all_sizes(new_size);
}

Widget *Button::notify_impl(Event ev)
{
	if (handle_mouse_hover_events(ev))
		return this;
	if (handle_mouse_press_events(ev))
		return this;

	if (ev.type == EventType::MouseClick) {
		on_click(ev.window, *this);
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

	draw_rounded_rect(Point(), get_size(), ELEMENT_ROUNDNESS, color);
	label.draw();
}