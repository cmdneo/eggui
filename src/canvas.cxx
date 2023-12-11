#include <cassert>

#include "raylib/raylib.h"

#include "canvas.hxx"
#include "graphics.hxx"
#include "theme.hxx"

using namespace eggui;

// Canvas class members
//---------------------------------------------------------
Pen Canvas::acquire_pen()
{
	assert(active_pen_cnt == 0);
	return Pen(*this);
}

void Canvas::set_size(Point new_size)
{
	size = new_size;
	region_start = Point(0, 0);
	region_size = new_size;
}

void Canvas::set_draw_region(Point rect_start, Point rect_size)
{
	assert(is_box_inside_box(position, size, rect_start, rect_size));

	region_start = rect_start;
	region_size = rect_size;
}

// Pen class members
//---------------------------------------------------------
Pen::Pen(Canvas &canvas_)
	: canvas(canvas_)
{
	auto abs_pos = push_translation(canvas.position);
	BeginScissorMode(
		abs_pos.x, abs_pos.y, canvas.region_size.x, canvas.region_size.y
	);
	canvas.active_pen_cnt++;
}

Pen::~Pen()
{

	pop_translation();
	EndScissorMode();
	canvas.active_pen_cnt--;
}
