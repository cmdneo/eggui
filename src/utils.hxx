#ifndef UTILS_HXX_INCLUDED
#define UTILS_HXX_INCLUDED

#include "point.hxx"
#include "raylib/raylib.h"

inline Rectangle points_to_rec(eggui::Point pos, eggui::Point size)
{
	Rectangle ret;
	ret.x = pos.x;
	ret.y = pos.y;
	ret.width = size.x;
	ret.height = size.y;

	return ret;
}

inline Rectangle shrink_from_center(Rectangle rect, float x, float y)
{
	rect.x += x;
	rect.y += y;
	rect.height -= 2 * x;
	rect.width -= 2 * y;
	return rect;
}

#endif