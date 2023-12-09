#include <algorithm>

#include "raylib/raylib.h"

#include "button.hxx"
#include "theme.hxx"
#include "utils.hxx"

using namespace eggui;

Widget *Button::notify(Event ev)
{
	if (handle_mouse_hover_events(ev))
		return this;
	if (handle_mouse_press_events(ev))
		return this;

	if (ev.type == EventType::MouseClick) {
		on_click(*this);
		return this;
	}

	return Interactive::notify(ev);
};

void Button::draw()
{
	auto color = BUTTON_COLOR;
	if (is_pressed)
		color = BUTTON_CLICK_COLOR;
	else if (is_hovering)
		color = BUTTON_HOVER_COLOR;

	auto rect = points_to_rec(get_position(), get_size());
	auto segs = std::min(4, get_size().min_coord() / 4);
	DrawRectangleRounded(rect, 0.25, segs, color);

	// Draw centered text
	auto text_size = MeasureTextEx(EG_MONO_FONT, label, EG_FONT_SIZE, 0);
	// Text must be drawn at integer coords, otherwise it does not get rendered properly
	int posx = rect.x + (rect.width - text_size.x) / 2;
	int posy = rect.y + (rect.height - EG_FONT_SIZE) / 2;
	DrawTextEx(
		EG_MONO_FONT, label, Vector2{1.f * posx, 1.f * posy}, EG_FONT_SIZE, 0,
		TEXT_COLOR
	);
}