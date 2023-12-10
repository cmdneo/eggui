#include <cassert>
#include <algorithm>
#include <memory>
#include <ranges>
#include <utility>
#include <functional>
#include <type_traits>

#include "widget.hxx"
#include "container.hxx"
#include "theme.hxx"
#include "point.hxx"
#include "graphics.hxx"

namespace ranges = std::ranges;

using namespace eggui;

using std::pair;
using std::unique_ptr;

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

// Container members
//---------------------------------------------------------
void Container::set_size(Point new_size)
{
	// We never keep a container a less than its minimum size
	// to ensure proper layout of elements, even if those elements lie outside
	// of the drawing[(0, 0) to screen_size] area.
	auto min_sz = calc_min_size();
	new_size.x = std::max(min_sz.x, new_size.x);
	new_size.y = std::max(min_sz.y, new_size.y);
	Widget::set_size(new_size);
}

// LinearBox members
//---------------------------------------------------------
Widget *LinearBox::add_widget_start(unique_ptr<Widget> child)
{
	auto ret = child.get();
	child->set_parent(this);
	start_children.push_back(std::move(child));
	return ret;
}

Widget *LinearBox::add_widget_end(unique_ptr<Widget> child)
{
	auto ret = child.get();
	child->set_parent(this);
	end_children.push_front(std::move(child));
	return ret;
}

void LinearBox::layout_children()
{
	const int count = start_children.size() + end_children.size();
	cell_offsets.resize(count);
	cell_sizes.resize(count);
	ranges::fill(cell_offsets, 0);
	ranges::fill(cell_sizes, 0);

	// Get x or y of the point depending on the orientation we are in.
	// In horizontal we want height and in vertical we want width.

	std::function<int(Point)> get_ocoord;
	if (orientation == Orientation::Horizontal)
		get_ocoord = [](Point pt) { return pt.y; };
	else
		get_ocoord = [](Point pt) { return pt.x; };

	// auto size_it = cell_sizes.begin();
	// Calculate size of each cell along with the
	// offsets for the columns(orient=H) or rows(orient=V).
	ranges::for_each((start_children), [&](auto &c) {
		max_cell_size = std::max(max_cell_size, get_ocoord(c->get_size()));
	});
}

Widget *LinearBox::notify(Event) { return nullptr; }

void LinearBox::set_position(Point) {}

void LinearBox::draw_debug() { Container::draw_debug(); }

void LinearBox::draw() {}

// Grid members
//---------------------------------------------------------
Widget *Grid::add_widget_beside(
	unique_ptr<Widget> child, const Widget *beside, Direction stick,
	int column_span, int row_span
)
{
	assert(!child->get_parent());
	assert(beside);
	assert(row_span > 0);
	assert(column_span > 0);

	auto sibling = std::ranges::find_if(children, [beside](Child &c) {
		return c.widget.get() == beside;
	});
	if (sibling == children.end())
		return nullptr;

	auto has_flag = [](Direction flags, Direction flag) {
		using T = std::underlying_type_t<Direction>;
		auto uflag = static_cast<T>(flag);
		return (static_cast<T>(flags) & uflag) == uflag;
	};

	auto gpos = sibling->grid_pos;

	if (has_flag(stick, Direction::Top))
		gpos.y -= row_span;
	else if (has_flag(stick, Direction::Bottom))
		gpos.y += sibling->span.y;

	if (has_flag(stick, Direction::Left))
		gpos.x += sibling->span.x;
	else if (has_flag(stick, Direction::Right))
		gpos.x -= column_span;

	if (gpos.x < 0 || gpos.y < 0)
		return nullptr;

	return add_widget(std::move(child), gpos.x, gpos.y, column_span, row_span);
}

Widget *Grid::add_widget(
	unique_ptr<Widget> child, int column, int row, int column_span, int row_span
)
{
	assert(!child->get_parent());
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
	child->set_parent(this);

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
	row_sizes.resize(row_count);
	col_sizes.resize(col_count);
	row_offsets.resize(row_count);
	col_offsets.resize(col_count);
	ranges::fill(row_sizes, 0);
	ranges::fill(col_sizes, 0);
	ranges::fill(row_offsets, 0);
	ranges::fill(col_offsets, 0);

	auto get_child_range = [](Child &c) {
		return std::pair(c.grid_pos, c.grid_pos + c.span);
	};

	// First we layout all of its children which are Containers.
	// So that they get their correct size and position.
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
	Widget::set_size(grid_size);

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
		Point pos(col_offsets[c.grid_pos.x], row_offsets[c.grid_pos.y]);
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

void Grid::draw_debug()
{
	ACQUIRE_CLEARED_CLEAR();

	const RGBA PINK(255, 109, 192);
	auto start = get_position();
	auto end = start + get_size();

	// Draw each row and column line, also draw the gap between them.
	for (int i = 0; i < col_count; ++i) {
		Point p1 = start + Point(col_offsets[i], 0);
		Point p2 = Point(p1.x, end.y);
		draw_line(p1, p2, PINK);
		p1.x += col_sizes[i];
		p2.x += col_sizes[i];
		draw_line(p1, p2, PINK);
	}

	for (int i = 0; i < row_count; ++i) {
		Point p1 = start + Point(0, row_offsets[i]);
		Point p2 = Point(end.x, p1.y);
		draw_line(p1, p2, PINK);
		p1.x += row_sizes[i];
		p1.x += row_sizes[i];
		draw_line(p1, p2, PINK);
	}

	for (auto &c : children)
		c.widget->draw_debug();
}

void Grid::draw()
{
	ACQUIRE_CLEARED_CLEAR();

	for (auto &c : children)
		c.widget->draw();
}
