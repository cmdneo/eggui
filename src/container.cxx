#include <cassert>
#include <algorithm>
#include <memory>
#include <numeric>
#include <ranges>
#include <utility>
#include <functional>
#include <type_traits>

#include "widget.hxx"
#include "container.hxx"
#include "theme.hxx"
#include "graphics.hxx"

namespace ranges = std::ranges;

using namespace eggui;

using std::pair;
using std::unique_ptr;
using std::vector;

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

/// @brief Calculates stretched size of the box within another box
/// @param min_size Minimum size of the inner box
/// @param max_size Maximum size of the inner box
/// @param avail_size Size of the outer box
/// @param fill Fill mode for stretching
/// @return Calculated size
Point calc_stretched_size(
	Point min_size, Point max_size, Point avail_size, Fill fill
)
{
	assert(min_size.x <= avail_size.x);
	assert(min_size.y <= avail_size.y);

	avail_size.x = std::min(max_size.x, avail_size.x);
	avail_size.y = std::min(max_size.y, avail_size.y);

	switch (fill) {
	case Fill::StretchColumn:
		return Point(avail_size.x, min_size.y);
	case Fill::StretchRow:
		return Point(min_size.x, avail_size.y);
	case Fill::Stretch:
		return avail_size;
	case Fill::None:
		return min_size;
	}

	assert(!"unreachable");
	return Point(0, 0);
}

/// @brief Calculates offsets of boxes for layout.
/// @param sizes Length of each box.
/// @param gap Gap between boxes along the length.
/// @param result Resulting offsets of boxes from 0.
/// @return Total length of the container containing the boxes.
float calc_box_offsets(
	const vector<float> &sizes, float gap, vector<float> &result
)
{
	result.resize(sizes.size());

	double last_at = 0.;
	for (auto i = 0u; i != sizes.size(); ++i) {
		result[i] = last_at;
		last_at += sizes[i] + gap;
	}
	last_at -= gap;

	return last_at;
}

// Container members
//---------------------------------------------------------
void Container::set_size(Point new_size)
{
	// Size is set after we are done laying out the children
	calc_layout_info();
	layout_children(new_size);
	// if (size.x > new_size.x || size.y > new_size.y) {
	// 	// TODO Warn that size is too small
	// }
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

void LinearBox::layout_children(Point)
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

Widget *LinearBox::notify_impl(Event) { return nullptr; }

void LinearBox::set_position(Point) {}

void LinearBox::draw_debug_impl() { Container::draw_debug_impl(); }

void LinearBox::draw_impl() {}

// Grid members
//---------------------------------------------------------
Widget *Grid::add_widget_beside(
	unique_ptr<Widget> child, const Widget *beside, Direction stick,
	int column_span, int row_span, Fill fill

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

	return add_widget(
		std::move(child), gpos.x, gpos.y, column_span, row_span, fill
	);
}

Widget *Grid::add_widget(
	unique_ptr<Widget> child, int column, int row, int column_span,
	int row_span, Fill fill
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
		.fill = fill
	});

	return ret_ptr;
}

Widget *Grid::notify_impl(Event ev)
{
	for (auto &c : children) {
		if (!c.widget->collides_with_point(ev.cursor))
			continue;
		return c.widget->notify(ev);
	}

	return nullptr;
}

void Grid::draw_debug_impl()
{
	constexpr RGBA PINK(255, 109, 192);
	constexpr RGBA TRANS_PINK(255, 109, 192, 32);

	// Draw each row and column line, also draw the gap between them.
	// We draw_rect_lines because draw_line produces a dim line, IDK why.
	for (int i = 0; i < col_count; ++i) {
		Point start(col_offsets[i], 0);
		Point size(col_sizes[i], get_size().y);
		draw_rect_lines(start, size, PINK);

		// Fill the gap
		start += Point(size.x, 0);
		if (col_gap != 0 && i != col_count - 1)
			draw_rect(start, Point(col_gap, get_size().y), TRANS_PINK);
	}

	for (int i = 0; i < row_count; ++i) {
		Point start(0, row_offsets[i]);
		Point size(get_size().x, row_sizes[i]);
		draw_rect_lines(start, size, PINK);

		// Fill the gap
		start += Point(0, size.y);
		if (row_gap != 0 && i != row_count - 1)
			draw_rect(start, Point(get_size().x, row_gap), TRANS_PINK);
	}

	// Draw the outer box over grid lines, since lines are drawn as rectangles.
	Widget::draw_debug_impl();

	for (auto &c : children)
		c.widget->draw_debug();
}

void Grid::draw_impl()
{
	for (auto &c : children)
		c.widget->draw();
}

void Grid::layout_children(Point size_hint)
{
	assert(row_count == int(row_sizes.size()));
	assert(col_count == int(col_sizes.size()));

	// We never go beyond the maximum size of the container
	size_hint.x = std::min(size_hint.x, get_max_size().x);
	size_hint.y = std::min(size_hint.y, get_max_size().y);

	// Expand if more space is available
	// If available space is less than the minimum space then do not shrink
	// anything, just draw the widgets. Oveflowing figures will get clipped.
	Point extra_size = size_hint - get_min_size();
	// We only expand the cells, not the gaps.
	Point gaps_size((col_count - 1) * col_gap, (row_count - 1) * row_gap);
	Point gapless_size = get_min_size() - gaps_size;
	double x_inc = std::max(0., 1. * extra_size.x / gapless_size.x);
	double y_inc = std::max(0., 1. * extra_size.y / gapless_size.y);

	// Calculate sizes of columns and rows
	for (auto i = 0; i < col_count; ++i)
		col_sizes[i] = (1. + x_inc) * col_min_sizes[i];
	for (auto i = 0; i < row_count; ++i)
		row_sizes[i] = (1. + y_inc) * row_min_sizes[i];

	// Calculate offsets
	Point cont_size(
		calc_box_offsets(col_sizes, col_gap, col_offsets),
		calc_box_offsets(row_sizes, row_gap, row_offsets)
	);

	// Calculate the size available for each child for alignment purposes.
	// Cell(s) may become larger than the widget due to another widget in the
	// same row/column having larger size.
	// Then stretch accordingly and layout each child.
	for (auto &c : children) {
		auto [start, end] = pair(c.grid_pos, c.grid_pos + c.span);
		end -= Point(1, 1);
		Point avail_size(
			col_offsets[end.x] - col_offsets[start.x] + col_sizes[end.x],
			row_offsets[end.y] - row_offsets[start.y] + row_sizes[end.y]
		);

		auto size = calc_stretched_size(
			c.widget->get_min_size(), c.widget->get_max_size(), avail_size,
			c.fill
		);
		c.widget->set_size(size);

		Point pos(col_offsets[c.grid_pos.x], row_offsets[c.grid_pos.y]);
		pos += calc_align_offset(
			avail_size, c.widget->get_size(), c.h_align, c.v_align
		);
		c.widget->set_position(pos);
	}

	Widget::set_size(cont_size);
}

Point Grid::calc_layout_info()
{
	alloc_row_col_data();

	// Calculate minimum and maximum size of each row and column.
	// If a widget spans multiple cells then, we also need to consider the
	// gaps between each cell for calculating cell size, since it will span
	// those gaps too. For that we simply remove the amount of size it spans
	// on gaps and only use the remaining size for cells.
	for (auto &c : children) {
		// We do layout calculation for inner containers.
		auto cont = dynamic_cast<Container *>(c.widget.get());
		if (cont)
			cont->calc_layout_info();

		auto max_size = c.widget->get_max_size();
		auto min_size = c.widget->get_min_size();

		Point gap(col_gap, row_gap);
		auto gap_taken = mul_components(c.span - Point(1, 1), gap);
		max_size -= gap_taken;
		min_size -= gap_taken;

		float cell_w_min = 1. * min_size.x / c.span.x;
		float cell_h_min = 1. * min_size.y / c.span.y;

		float cell_w_max = 1. * max_size.x / c.span.x;
		float cell_h_max = 1. * max_size.y / c.span.y;

		auto [start, end] = pair(c.grid_pos, c.grid_pos + c.span);
		for (int i = start.x; i < end.x; ++i) {
			col_min_sizes[i] = std::max(col_min_sizes[i], cell_w_min);
			col_max_sizes[i] = std::max(col_max_sizes[i], cell_w_max);
		}
		for (int i = start.y; i < end.y; ++i) {
			row_min_sizes[i] = std::max(row_min_sizes[i], cell_h_min);
			row_max_sizes[i] = std::max(row_max_sizes[i], cell_h_max);
		}
	}

	auto calc_gapped_size = [](const vector<float> &ns, int gap) {
		float sum = 0;
		ranges::for_each(ns, [&sum](float n) { sum += n; });
		return sum + gap * (ns.size() - 1);
	};

	Point min_size(
		calc_gapped_size(col_min_sizes, col_gap),
		calc_gapped_size(row_min_sizes, row_gap)
	);
	Point max_size(
		calc_gapped_size(col_max_sizes, col_gap),
		calc_gapped_size(row_max_sizes, row_gap)
	);

	set_min_size(min_size);
	set_max_size(max_size);
	return min_size;
}

void Grid::alloc_row_col_data()
{
	row_sizes.resize(row_count);
	ranges::fill(row_sizes, 0);

	col_sizes.resize(col_count);
	ranges::fill(col_sizes, 0);

	row_max_sizes.resize(row_count);
	ranges::fill(row_min_sizes, 0);

	col_max_sizes.resize(col_count);
	ranges::fill(col_min_sizes, 0);

	row_min_sizes.resize(row_count);
	ranges::fill(row_max_sizes, 0);

	col_min_sizes.resize(col_count);
	ranges::fill(col_max_sizes, 0);

	row_offsets.resize(row_count);
	ranges::fill(row_offsets, 0);

	col_offsets.resize(col_count);
	ranges::fill(col_offsets, 0);
}