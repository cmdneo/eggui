#include <cassert>
#include <algorithm>
#include <memory>

#include "widget.hxx"
#include "container.hxx"
#include "theme.hxx"
#include "raylib/raylib.h"

using namespace eggui;

enum class FillMode { Column, Row };

/// @brief Puts the box inside a rectangular container.
/// @param free_start Container free start point(inclusive).
/// @param free_end   Container free end point(excluded).
/// @param box_pos    Box postion result arg.
/// @param box_size   Box size.
/// @param corner     Anchor direction, one of the four corners.
/// @param fill       Fill mode: Column(vertical) or Row(horizontal).
/// @return True if box fits, false otherwise and do nothing.
///
/// @details Puts the box inside a rectanglular container represented by
/// `start` and `end` and if the box fits inside it, then updates `start`
/// and `end` to reflect their new free space.
/// Puts postion assigned to the box in `box_pos`.
bool put_box_in_container(
	Point &free_start, Point &free_end, Point &box_pos, Point box_size,
	Anchor corner, FillMode fill
)
{
	auto size = box_size;
	auto start = free_start;
	auto end = free_end;

	// Box positions for each anchor corner in order: TL, TR, BL, BR.
	Point all_positions[] = {
		start,
		Point(end.x - size.x, start.y),
		Point(start.x, end.y - size.y),
		end - size,
	};

	if (fill == FillMode::Row) {
		switch (corner) {
		case Anchor::TopLeft:
		case Anchor::TopRight:
			start.y += size.y;
			break;

		case Anchor::BottomLeft:
		case Anchor::BottomRight:
			end.y -= size.y;
			break;
		}
	} else {
		switch (corner) {
		case Anchor::TopLeft:
		case Anchor::BottomLeft:
			start.x += size.x;
			break;

		case Anchor::TopRight:
		case Anchor::BottomRight:
			end.x -= size.x;
			break;
		}
	}

	auto space_left = end - start;
	if (space_left.x < 0 || space_left.y < 0)
		return false;

	// Update only if the box fits
	box_pos = all_positions[int(corner)];
	free_start = start;
	free_end = end;

	return true;
}

void Container::draw_debug()
{
	Widget::draw_debug();

	auto size = free_end - free_start;
	if (size.x > 0)
		DrawRectangle(
			free_start.x + position.x, free_start.y + position.y, size.x,
			size.y, {0, 255, 0, 16}
		);
}

bool Container::add_widget(Anchor anchor, std::unique_ptr<Widget> w)
{
	w->anchor_corner = anchor;
	children.push_back(std::move(w));
	return true;
}

void Container::set_size(int width, int height)
{
	box_size = Point(width, height);
	free_start = {};
	free_end = box_size;
	layout_children();
}

void HorizontalContainer::layout_children()
{
	Point box_pos{};

	for (auto &c : children) {
		put_box_in_container(
			free_start, free_end, box_pos, c->get_size(), c->anchor_corner,
			FillMode::Column
		);
		c->set_position(box_pos);
	}
}

void VerticalContainer::layout_children()
{
	Point box_pos{};

	for (auto &c : children) {
		put_box_in_container(
			free_start, free_end, box_pos, c->get_size(), c->anchor_corner,
			FillMode::Row
		);
		c->set_position(box_pos);
	}
}