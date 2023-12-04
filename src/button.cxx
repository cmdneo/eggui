#include <algorithm>

#include "raylib/raylib.h"

#include "button.hxx"
#include "theme.hxx"
#include "utils.hxx"

using namespace eggui;

Widget *Button::notify(Event ev)
{
	
	switch (ev.type) {
	case EventType::MouseIn:
		is_hovering = true;
		break;
	case EventType::MouseOut:
		is_hovering = false;
		break;

	case EventType::MousePressed:
		is_pressed = true;
		break;
	case EventType::MouseReleased:
		is_pressed = false;
		break;

	case EventType::MouseClick:
		click_action(*this);
		break;

	default:
		return Interactive::notify(ev);
	}

	return this;
};

Point margin(5, 5);

void Button::draw()
{
	auto bg = BUTTON_COLOR;

	if (is_pressed)
		bg = BUTTON_CLICK_COLOR;
	else if (is_hovering)
		bg = BUTTON_HOVER_COLOR;

	Rectangle rect =
		points_to_rec(get_position() + margin, get_size() - margin - margin);
	DrawRectangleRounded(rect, 0.25, 10, bg);

	// Draw centered text
	auto text_size = MeasureTextEx(EG_MONO_FONT, label, EG_FONT_SIZE, 0);
	float posx = rect.x + (rect.width - text_size.x) / 2;
	float posy = rect.y + (rect.height - EG_FONT_SIZE) / 2;
	DrawTextEx(
		EG_MONO_FONT, label, Vector2{posx, posy}, EG_FONT_SIZE, 0, TEXT_COLOR
	);
}