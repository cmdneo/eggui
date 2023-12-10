#include <cassert>

#include "raylib/raylib.h"
#include "canvas.hxx"
#include "managers.hxx"
#include "graphics.hxx"
#include "theme.hxx"

using namespace eggui;

inline Rectangle points_to_rect(Point pos, Point size)
{
	Rectangle ret;
	ret.x = pos.x;
	ret.y = pos.y;
	ret.width = size.x;
	ret.height = size.y;

	return ret;
}

// Canvas class members
//---------------------------------------------------------
Canvas::Canvas(Point position_, Point size_)
	: size(size_)
	, position(position_)
	, region_start(Point(0, 0))
	, region_size(size_)
{
	if (size.x == 0 || size.y == 0)
		return;
	texture_id = TextureManager::instance().create_texture(size);
}

Canvas::~Canvas()
{
	// The texture was never allocated
	if (texture_id == -1)
		return;
	TextureManager::instance().destroy_texture(texture_id);
}

Pen Canvas::acquire_pen()
{
	assert(!has_active_pen);
	return Pen(*this);
}

void Canvas::resize_texture(Point new_size)
{
	if (texture_id != -1)
		TextureManager::instance().destroy_texture(texture_id);

	texture_id = TextureManager::instance().create_texture(new_size);
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
	// Canvas should have its texture allocated for drawing.
	assert(canvas.texture_id != -1);

	canvas_.has_active_pen = true;
	BeginTextureMode(TextureManager::instance().get(canvas.texture_id));
}

Pen::~Pen()
{
	auto rect = points_to_rect(canvas.region_start, canvas.region_size);
	rect.height = -rect.height; // Flip vertically
	Vector2 pos = {1.f * canvas.position.x, 1.f * canvas.position.y};

	EndTextureMode();

	DrawTextureRec(
		TextureManager::instance().get(canvas.texture_id).texture, rect, pos,
		WHITE
	);

	canvas.has_active_pen = false;
}
