#include <cassert>
#include <algorithm>
#include <memory>
#include <ranges>
#include <utility>
#include <type_traits>

#include "raylib/raylib.h"

#include "widget.hxx"
#include "container.hxx"
#include "theme.hxx"
#include "point.hxx"

using namespace eggui;

enum class FillMode { Column, Row };

/// @brief Calculates the relative-position of child inside contatiner as per
/// horinzontal and vertical alingments.
Point calc_align_offset(
	Point cont_size, Point child_size, Alignment halign, Alignment valign
)
{
	auto align_pos = [](int outer, int inner, Alignment align) -> int {
		switch (align) {
		case Alignment::Start:
			return 0;
		case Alignment::Center:
			return (outer - inner) / 2;
		case Alignment::End:
			return outer - inner;
		default:
			assert(!"Unreachable");
		}
	};

	return Point(
		align_pos(cont_size.x, child_size.x, halign),
		align_pos(cont_size.y, child_size.y, valign)
	);
}

// Point put_box_in_container(
// 	Point &free_start, Point &free_end, Point box_size, Anchor corner,
// 	FillMode fill
// )
// {
// 	auto size = box_size;
// 	auto start = free_start;
// 	auto end = free_end;

// 	// Box positions for each anchor corner in order: TL, TR, BL, BR.
// 	Point all_positions[] = {
// 		start,
// 		Point(end.x - size.x, start.y),
// 		Point(start.x, end.y - size.y),
// 		end - size,
// 	};

// 	if (fill == FillMode::Row) {
// 		switch (corner) {
// 		case Anchor::TopLeft:
// 		case Anchor::TopRight:
// 			start.y += size.y;
// 			break;

// 		case Anchor::BottomLeft:
// 		case Anchor::BottomRight:
// 			end.y -= size.y;
// 			break;
// 		}
// 	} else {
// 		switch (corner) {
// 		case Anchor::TopLeft:
// 		case Anchor::BottomLeft:
// 			start.x += size.x;
// 			break;

// 		case Anchor::TopRight:
// 		case Anchor::BottomRight:
// 			end.x -= size.x;
// 			break;
// 		}
// 	}

// 	auto space_left = end - start;
// 	if (space_left.x < 0 || space_left.y < 0) {
// 		TraceLog(LOG_FATAL, "%s: Box does not fit.", __func__);
// 		abort();
// 	}

// 	// Update only if the box fits
// 	free_start = start;
// 	free_end = end;
// 	return all_positions[int(corner)];
// }

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

void Container::set_size(int width, int height)
{
	// We never keep a container a less than its minimum size
	// to ensure proper layout of elements, even if those elements lie outside
	// of the drawing[(0, 0) to screen_size] area.
	auto min_sz = calc_min_size();
	width = std::max(min_sz.x, width);
	height = std::max(min_sz.y, height);
	Widget::set_size(width, height);

	// A layout calculation is needed after resize
	free_start = get_position();
	free_end = get_position() + get_size();
}

// void HorizontalContainer::layout_child_widget(Widget &child)
// {
// 	// We set first set the position then set size so that
// 	// layout calcualations work properly.
// 	auto size = child.calc_min_size();
// 	auto pos = put_box_in_container(
// 		free_start, free_end, size, child.anchor_corner, FillMode::Column
// 	);
// 	child.set_position(pos);

// 	// If it is a container then we need stretch and set its size.
// 	// Strech and fill column if smaller.
// 	auto cont = dynamic_cast<Container *>(&child);
// 	if (cont) {
// 		cont->set_ypos(get_position().y);
// 		cont->set_size(size.x, std::max(get_size().y, size.y));
// 	}
// }

// Point HorizontalContainer::calc_min_size()
// {
// 	Point min_sz(0, 0);

// 	for (auto &c : children) {
// 		auto sz = c->calc_min_size();
// 		min_sz.x += sz.x;
// 		min_sz.y = std::max(min_sz.y, sz.y);
// 	}

// 	return min_sz;
// }

// void VerticalContainer::layout_child_widget(Widget &child)
// {
// 	// We set first set the position then set size so that
// 	// layout calcualations work properly.
// 	auto size = child.calc_min_size();
// 	auto pos = put_box_in_container(
// 		free_start, free_end, size, child.anchor_corner, FillMode::Row
// 	);
// 	child.set_position(pos);

// 	// If it is a container then we need stretch and set its size.
// 	// Strech and fill row if smaller.
// 	auto cont = dynamic_cast<Container *>(&child);
// 	if (cont) {
// 		cont->set_xpos(get_position().x);
// 		cont->set_size(std::max(get_size().x, size.x), size.y);
// 	}
// }

// Point VerticalContainer::calc_min_size()
// {
// 	Point min_sz(0, 0);

// 	for (auto &c : children) {
// 		auto sz = c->calc_min_size();
// 		min_sz.x = std::max(min_sz.x, sz.x);
// 		min_sz.y += sz.y;
// 	}

// 	return min_sz;
// }

Widget *Grid::add_widget_beside(
	std::unique_ptr<Widget> child, const Widget *beside, Direction stick,
	int column_span, int row_span
)
{
	assert(beside);
	assert(row_span > 0);
	assert(column_span > 0);

	auto sibling = std::ranges::find_if(children, [beside](Child &c) {
		return c.widget.get() == beside;
	});
	if (sibling == children.end())
		return nullptr;

	auto has_bit = [](Direction flags, Direction flag) {
		using T = std::underlying_type_t<Direction>;
		return static_cast<T>(flags) & static_cast<T>(flag);
	};

	auto gpos = sibling->grid_pos;

	if (has_bit(stick, Direction::Top))
		gpos.y -= row_span;
	else if (has_bit(stick, Direction::Bottom))
		gpos.y += sibling->span.y;

	if (has_bit(stick, Direction::Left))
		gpos.x += sibling->span.x;
	else if (has_bit(stick, Direction::Right))
		gpos.x -= column_span;

	return add_widget(std::move(child), gpos.x, gpos.y, column_span, row_span);
}

Widget *Grid::add_widget(
	std::unique_ptr<Widget> child, int column, int row, int column_span,
	int row_span
)
{
	assert(row >= 0);
	assert(column >= 0);
	assert(row_span > 0);
	assert(column_span > 0);

	Point pos(column, row);
	Point span(column_span, row_span);

	for (auto &c : children) {
		if (check_box_collision(pos, span, c.grid_pos, c.span))
			return nullptr;
	}

	auto end = pos + span;
	if (end.x > col_count)
		col_count = end.x;
	if (end.y > row_count)
		row_count = end.y;

	auto ret_ptr = child.get();
	children.push_back(Child{
		.widget = std::move(child),
		.grid_pos = pos,
		.span = span,
		.h_align = Alignment::Center,
		.v_align = Alignment::Center,
	});

	return ret_ptr;
}

void Grid::layout_children()
{
	using std::ranges::fill;

	row_sizes.resize(row_count);
	col_sizes.resize(col_count);
	row_offsets.resize(row_count);
	col_offsets.resize(col_count);
	fill(row_sizes, 0);
	fill(col_sizes, 0);
	fill(row_offsets, 0);
	fill(col_offsets, 0);

	auto get_child_range = [](Child &c) {
		return std::pair(c.grid_pos, c.grid_pos + c.span);
	};

	// First we layout all of its children which are Containers.
	// So that the get their correct size and position.
	for (auto &c : children) {
		auto cont = dynamic_cast<Container *>(c.widget.get());
		if (cont)
			cont->layout_children();
	}

	// Calculate minimum size of each row and column.
	// If a widget spans multiple cells then, we also need to consider the
	// gaps between each cell for calculating cell size, since it will span
	// those gaps too. For that we simply remove the amount of size it spans
	// on gaps and only use the remaining size for cells.
	for (auto &c : children) {
		auto size = c.widget->get_size();
		size.x = size.x - (c.span.x - 1) * col_gap;
		size.y = size.y - (c.span.y - 1) * row_gap;
		float cell_w = 1. * (size.x) / c.span.x;
		float cell_h = 1. * size.y / c.span.y;

		auto [start, end] = get_child_range(c);
		for (int i = start.x; i < end.x; ++i)
			col_sizes[i] = std::max(col_sizes[i], cell_w);
		for (int i = start.y; i < end.y; ++i)
			row_sizes[i] = std::max(row_sizes[i], cell_h);
	}

	// Calculate offset of each column
	double last_x = 0;
	for (auto i = 0; i < col_count; ++i) {
		col_offsets[i] = last_x;
		last_x += col_sizes[i] + col_gap;
	}
	// Calculate offset of each row
	double last_y = 0;
	for (auto i = 0; i < row_count; ++i) {
		row_offsets[i] = last_y;
		last_y += row_sizes[i] + row_gap;
	}

	// Remove the gap after the last row and column.
	Point grid_size(last_x - col_gap, last_y - row_gap);
	Widget::set_size(grid_size.x, grid_size.y);

	// Calculate size available to each children for alingment purposes.
	// Cell(s) may become larger than the child it holds due to another
	// widget in the same row/column having larger size.
	for (auto &c : children) {
		auto [start, end] = get_child_range(c);
		end -= Point(1, 1);
		c.avail_size = Point(
			col_offsets[end.x] - col_offsets[start.x] + col_sizes[end.x],
			row_offsets[end.y] - row_offsets[start.y] + row_sizes[end.y]
		);
	}

	// Finally layout all the children
	for (auto &c : children) {
		int posx = col_offsets[c.grid_pos.x];
		int posy = row_offsets[c.grid_pos.y];
		auto pos = Point(posx, posy) + get_position();

		pos += calc_align_offset(
			c.avail_size, c.widget->get_size(), c.h_align, c.v_align
		);
		c.widget->set_position(pos);
	}
}

Widget *Grid::notify(Event ev)
{
	for (auto &c : children) {
		if (!c.widget->collides_with_point(ev.cursor))
			continue;
		return c.widget->notify(ev);
	}

	return nullptr;
}

void Grid::set_position(Point new_pos)
{
	auto delta = new_pos - get_position();
	Widget::set_position(new_pos);

	for (auto &c : children)
		c.widget->set_position(c.widget->get_position() + delta);
}

void Grid::set_size(int width, int height) { Widget::set_size(width, height); }

void Grid::draw_debug()
{
	Widget::draw_debug();

	auto start = get_position();
	auto end = start + get_size();

	// Draw each row and column line, also draw the gap between them.
	for (int i = 0; i < col_count; ++i) {
		int px = start.x + col_offsets[i];
		DrawLine(px, start.y, px, end.y, PINK);
		px += col_sizes[i];
		DrawLine(px, start.y, px, end.y, PINK);
	}

	for (int i = 0; i < row_count; ++i) {
		int py = start.y + row_offsets[i];
		DrawLine(start.x, py, end.x, py, PINK);
		py += row_sizes[i];
		DrawLine(start.x, py, end.x, py, PINK);
	}

	for (auto &c : children)
		c.widget->draw_debug();
}

void Grid::draw()
{
	for (auto &c : children)
		c.widget->draw();
}
