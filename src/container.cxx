#include <cassert>
#include <algorithm>
#include <memory>
#include <optional>

#include "widget.hxx"
#include "container.hxx"
#include "theme.hxx"
#include "raylib/raylib.h"

using namespace eggui;

enum class FillMode { Column, Row };

/// @brief Puts the box inside a rectangular container.
/// @param free_start Container free start point(inclusive).
/// @param free_end   Container free end point(excluded).
/// @param box_size   Box size.
/// @param corner     Anchor direction, one of the four corners.
/// @param fill       Fill mode: Column(vertical) or Row(horizontal).
/// @return Retuns box position if box fits, otherwise nullopt.
///
/// @details Puts the box inside a rectanglular container represented by
/// `start` and `end` and if the box fits inside it, then updates `start`
/// and `end` to reflect their new free space.
std::optional<Point> put_box_in_container(
	Point &free_start, Point &free_end, Point box_size, Anchor corner,
	FillMode fill
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
		return std::nullopt;

	// Update only if the box fits
	free_start = start;
	free_end = end;
	return all_positions[int(corner)];
}

void Container::draw_debug()
{
	Widget::draw_debug();

	auto size = free_end - free_start;
	if (size.x > 0 && size.y > 0)
		DrawRectangle(
			free_start.x, free_start.y, size.x, size.y, {0, 255, 0, 16}
		);

	for (auto &c : children)
		c->draw_debug();
}

void Container::add_widget(Anchor corner, std::unique_ptr<Widget> w)
{
	w->anchor_corner = corner;
	children.push_back(std::move(w));
}

void Container::set_size(int width, int height)
{
	Widget::set_size(width, height);
	free_start = get_position();
	free_end = get_position() + get_size();

	for (auto &c : children)
		layout_child_widget(*c);
}

void HorizontalContainer::layout_child_widget(Widget &child)
{
	// We set first set the position then set size so that
	// layout calcualations work properly.
	auto size = child.calc_min_size();
	auto pos = put_box_in_container(
		free_start, free_end, size, child.anchor_corner, FillMode::Column
	);
	if (pos)
		child.set_position(*pos);
	
	// If is a container then propogate resize to it.
	// And if strechable then fill the entire column.`
	auto cont = dynamic_cast<Container *>(&child);
	if (cont && cont->is_strechable) {
		cont->set_ypos(get_position().y);
		cont->set_size(size.x, std::max(get_size().y, size.y));
	}
}

Point HorizontalContainer::calc_min_size()
{
	Point min_sz(0, 0);

	for (auto &c : children) {
		auto sz = c->calc_min_size();
		min_sz.x += sz.x;
		min_sz.y = std::max(min_sz.y, sz.y);
	}

	return min_sz;
}

void VerticalContainer::layout_child_widget(Widget &child)
{
	// We set first set the position then set size so that
	// layout calcualations work properly.
	auto size = child.calc_min_size();
	auto pos = put_box_in_container(
		free_start, free_end, size, child.anchor_corner, FillMode::Row
	);
	if (pos)
		child.set_position(*pos);

	// If is a container then propogate resize to it.
	// And if strechable then fill the entire row.`
	auto cont = dynamic_cast<Container *>(&child);
	if (cont && cont->is_strechable) {
		cont->set_xpos(get_position().x);
		cont->set_size(std::max(get_size().x, size.x), size.y);
	}
}

Point VerticalContainer::calc_min_size()
{
	Point min_sz(0, 0);

	for (auto &c : children) {
		auto sz = c->calc_min_size();
		min_sz.x = std::max(min_sz.x, sz.x);
		min_sz.y += sz.y;
	}

	return min_sz;
}
