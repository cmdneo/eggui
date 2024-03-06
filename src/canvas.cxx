#include <cassert>

#include "raylib/raylib.h"

#include "canvas.hxx"
#include "graphics.hxx"
#include "managers.hxx"
#include "theme.hxx"

using namespace eggui;

// Canvas class members
//---------------------------------------------------------
Pen Canvas::acquire_pen(bool enable_cliping)
{
	assert(active_pen_cnt == 0);
	return Pen(*this, enable_cliping);
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
Pen::Pen(Canvas &canvas_, bool is_clipping_)
	: canvas(canvas_)
	, is_clipping(is_clipping_)
{
	push_translation(canvas.position);
	canvas.active_pen_cnt++;

	auto start = get_total_translation() + canvas.region_start;
	auto size = canvas.region_size;

	auto [pos, sz] = ClippingManager::instance().calc_clip_area(start, size);
	clip_rect_pos = pos - get_total_translation(); // Make position relative
	clip_rect_size = sz;

	if (is_clipping)
		ClippingManager::instance().push_clip_area(start, size);
}

Pen::~Pen()
{
	pop_translation();
	canvas.active_pen_cnt--;

	if (is_clipping)
		ClippingManager::instance().pop_clip_area();
}
