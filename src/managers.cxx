#include <cassert>
#include <algorithm>
#include <memory>
#include <utility>

#include "raylib/raylib.h"

#include "point.hxx"
#include "managers.hxx"
#include "graphics.hxx"

using std::max;
using std::min;
using std::pair;
using std::unique_ptr;

using namespace eggui;

pair<Point, Point>
find_intersection(pair<Point, Point> rect1, pair<Point, Point> rect2)
{
	// Normalized rectangles, where a <= a1 and b <= b1.
	auto a = rect1.first;
	auto b = rect2.first;
	auto a1 = a + rect1.second;
	auto b1 = b + rect2.second;

	auto c = Point(max(a.x, b.x), max(a.y, b.y));
	auto c1 = Point(min(a1.x, b1.x), min(a1.y, b1.y));
	if (c1.x < c.x || c1.y < c.y)
		c1 = c;

	return pair(c, c1 - c);
}

// Clipping manager members
//---------------------------------------------------------
ClippingManager &ClippingManager::instance()
{
	static unique_ptr<ClippingManager> ptr(new ClippingManager());
	return *ptr;
}

void ClippingManager::push_clip_area(Point start, Point size)
{
	auto new_area = calc_clip_area(start, size);
	auto [pos, sz] = new_area;

	// Remove the previous area, we restore it when this new area is removed.
	if (!clip_areas.empty())
		EndScissorMode();

	BeginScissorMode(pos.x, pos.y, sz.x, sz.y);
	clip_areas.push_back(new_area);
}

void ClippingManager::pop_clip_area()
{
	assert(!clip_areas.empty());

	clip_areas.pop_back();
	EndScissorMode();

	// If a previous clip area was present then restore that.
	if (!clip_areas.empty()) {
		auto [pos, sz] = clip_areas.back();
		BeginScissorMode(pos.x, pos.y, sz.x, sz.y);
	}
}

pair<Point, Point> ClippingManager::get_current_clip_area() const
{
	if (clip_areas.empty())
		return pair(Point(0, 0), get_window_size());
	else
		return clip_areas.back();
}

pair<Point, Point>
ClippingManager::calc_clip_area(Point start, Point size) const
{
	auto new_area = pair(start, size);
	if (!clip_areas.empty())
		new_area = find_intersection(new_area, clip_areas.back());

	return new_area;
}

// Font manager members
//---------------------------------------------------------
FontManager &FontManager::instance()
{
	static unique_ptr<FontManager> ptr(new FontManager());
	return *ptr;
}