#include "theme.hxx"
#include "widget.hxx"
#include "raylib/raylib.h"

using namespace eggui;


void Widget::draw_debug()
{
	DrawRectangleLines(
		position.x, position.y, box_size.x, box_size.y, DEBUG_BORDER_COLOR
	);

	for (auto &c : children)
		c->draw_debug();
}