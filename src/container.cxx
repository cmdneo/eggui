#include <cassert>
#include <algorithm>
#include <memory>
#include <vector>
#include <ranges>
#include <utility>

#include "widget.hxx"
#include "container.hxx"
#include "graphics.hxx"
#include "theme.hxx"
#include "calc.hxx"

namespace ranges = std::ranges;
namespace views = std::views;

using namespace eggui;

using std::pair;
using std::vector;

// Container members
//---------------------------------------------------------
void Container::set_size(Point new_size)
{
	calc_layout_info();
	layout_children(new_size);
}

// PaddedBox members
//---------------------------------------------------------
void PaddedBox::set_padding(int top, int bottom, int left, int right)
{
	left_pad = left < 0 ? left_pad : left;
	right_pad = right < 0 ? right_pad : right;
	top_pad = top < 0 ? top_pad : top;
	bottom_pad = bottom < 0 ? bottom_pad : bottom;
}

void PaddedBox::layout_children(Point size_hint)
{
	size_hint = clamp_components(size_hint, get_min_size(), get_max_size());

	Point padding(left_pad + right_pad, top_pad + bottom_pad);
	auto avail_size = size_hint - padding;
	auto child_size = calc_stretched_size(
		child->get_min_size(), child->get_max_size(), avail_size,
		child->get_fill()
	);

	Point pos(left_pad, top_pad);
	auto cont_size = padding + child_size;

	child->set_position(pos);
	child->set_size(child_size);
	Widget::set_size(cont_size);
}

Point PaddedBox::calc_layout_info()
{
	calc_layout_info_if_container(*child);

	Point padding(left_pad + right_pad, top_pad + bottom_pad);
	set_min_size(child->get_min_size() + padding);
	set_max_size(child->get_max_size() + padding);
	return get_min_size();
}

Widget *PaddedBox::notify(Event ev)
{
	if (child->collides_with_point(ev.cursor))
		return notify_widget(*child, ev);
	return nullptr;
}

void PaddedBox::draw() { draw_widget(*child); }

void PaddedBox::draw_debug()
{
	Widget::draw_debug();
	draw_widget_debug(*child);
}

// LinearBox members
//---------------------------------------------------------
Widget *LinearBox::add_widget_start(std::shared_ptr<Widget> child)
{
	assert(!child->get_parent());
	child->set_parent(this);

	start_children.push_back(Child{
		.widget = std::move(child),
		.align = Alignment::Center,
		.fill = Fill::RowNColumn,
	});

	return start_children.back().widget.get();
}

Widget *LinearBox::add_widget_end(std::shared_ptr<Widget> child)
{
	assert(!child->get_parent());
	child->set_parent(this);

	end_children.push_back(Child{
		.widget = std::move(child),
		.align = Alignment::Center,
		.fill = Fill::RowNColumn,
	});

	return end_children.back().widget.get();
}

Widget *LinearBox::notify(Event ev)
{
	for (auto &c : start_children) {
		if (c.widget->collides_with_point(ev.cursor))
			return notify_widget(*c.widget, ev);
	}
	for (auto &c : end_children) {
		if (c.widget->collides_with_point(ev.cursor))
			return notify_widget(*c.widget, ev);
	}

	return nullptr;
}

void LinearBox::draw_debug()
{
	const int axis = static_cast<int>(orientation);

	// Draw grid lines and fill the gaps between them.
	for (unsigned i = 0; i < cell_sizes.size(); ++i) {
		Point start(0, 0);
		start[axis] = cell_offsets[i];

		Point size;
		size[axis] = cell_sizes[i];
		size[1 - axis] = get_size()[1 - axis];
		draw_rect_lines(start, size, GAP_COLOR);

		start[axis] += size[axis];
		if (item_gap == 0 || i == cell_sizes.size() - 1)
			continue;

		// Fill the gap if it exists for the cell.
		if (item_gap == 0)
			continue;
		Point gap_size;
		gap_size[axis] = item_gap;
		gap_size[1 - axis] = get_size()[1 - axis];
		draw_rect(start, gap_size, GAP_FILL_COLOR);
	}

	Container::draw_debug();

	for (auto &c : start_children)
		draw_widget_debug(*c.widget);
	for (auto &c : end_children)
		draw_widget_debug(*c.widget);
}

void LinearBox::draw()
{
	for (auto &c : start_children)
		draw_widget(*c.widget);
	for (auto &c : end_children)
		draw_widget(*c.widget);
}

void LinearBox::layout_children(Point size_hint)
{
	const auto count = start_children.size() + end_children.size();
	cell_sizes.reserve(count);
	cell_offsets.reserve(count);

	const int axis = static_cast<int>(orientation);
	size_hint = clamp_components(size_hint, get_min_size(), get_max_size());

	// Expand all the widgets with proportion to their expandable sizes.
	auto gapless_size = size_hint - calc_gap_size();
	calc_expanded_size(
		cell_min_sizes, cell_max_sizes, gapless_size[axis], cell_sizes
	);

	int last_at = calc_box_offsets(cell_sizes, item_gap, cell_offsets);

	// If the box still has space left after packing all the children then,
	// put that space between start and end children. Thereby start children
	// are at top/left and end children at bottom/right for orientations
	// vertical/horizontal.
	if (last_at < size_hint[axis] && !end_children.empty()) {
		auto len_extra = size_hint[axis] - last_at;
		ranges::for_each(
			cell_offsets | views::drop(start_children.size()),
			[len_extra](int &v) { v += len_extra; }
		);
	}

	// Calculates and set child position and size.
	auto calc_size_n_pos = [=, this](Child &c, int cell_index) {
		Widget &w = *c.widget;
		Point avail_size;
		avail_size[axis] = cell_sizes[cell_index];
		avail_size[1 - axis] = size_hint[1 - axis];

		auto size = calc_stretched_size(
			w.get_min_size(), w.get_max_size(), avail_size, c.fill
		);
		w.set_size(size);

		Point pos(0, 0);
		pos[axis] = cell_offsets[cell_index];
		pos += calc_align_offset(w.get_size(), avail_size, c.align, c.align);
		w.set_position(pos);
	};

	// Set position and size for each widget.
	int i = 0;
	for (auto &c : start_children)
		calc_size_n_pos(c, i++);
	for (auto &c : end_children | views::reverse)
		calc_size_n_pos(c, i++);

	Widget::set_size(size_hint);
}

Point LinearBox::calc_layout_info()
{
	Point max_size(0, 0);
	Point min_size(0, 0);

	const int axis = static_cast<int>(orientation);

	// Calculates size info for each widget
	auto calc_sizes = [&, this, axis](auto children_view) {
		for (auto &c : children_view) {
			calc_layout_info_if_container(*c.widget);
			max_size = max_components(max_size, c.widget->get_max_size());
			min_size = max_components(min_size, c.widget->get_min_size());
			cell_min_sizes.push_back(c.widget->get_min_size()[axis]);
			cell_max_sizes.push_back(c.widget->get_max_size()[axis]);
		}
	};

	cell_max_sizes.clear();
	cell_min_sizes.clear();
	calc_sizes(start_children | views::all);
	calc_sizes(end_children | views::reverse);

	int min_len = calc_length_with_gaps(cell_min_sizes, item_gap);
	int max_len = calc_length_with_gaps(cell_max_sizes, item_gap);

	if (expand_to_fill)
		max_len = UNLIMITED_MAX_SIZE;

	if (orientation == Orientation::Horizontal) {
		set_min_size(Point(min_len, min_size.y));
		set_max_size(Point(max_len, max_size.y));
	} else {
		set_min_size(Point(min_size.x, min_len));
		set_max_size(Point(max_size.x, max_len));
	}

	return get_min_size();
}

Point LinearBox::calc_gap_size() const
{
	auto g = item_gap * (start_children.size() + end_children.size() - 1);
	return Point(g, g);
}

// Grid members
//---------------------------------------------------------
Widget *Grid::add_widget_beside(
	std::shared_ptr<Widget> child, const Widget *beside, Direction stick,
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

	auto gpos = sibling->grid_pos;

	if (has_component(stick, Direction::Top))
		gpos.y -= row_span;
	else if (has_component(stick, Direction::Bottom))
		gpos.y += sibling->span.y;

	if (has_component(stick, Direction::Left))
		gpos.x += sibling->span.x;
	else if (has_component(stick, Direction::Right))
		gpos.x -= column_span;

	if (gpos.x < 0 || gpos.y < 0)
		return nullptr;

	return add_widget(std::move(child), gpos.x, gpos.y, column_span, row_span);
}

Widget *Grid::add_widget(
	std::shared_ptr<Widget> child, int column, int row, int column_span,
	int row_span
)
{
	assert(!child->get_parent());
	assert(row >= 0);
	assert(column >= 0);
	assert(row_span > 0);
	assert(column_span > 0);

	Point pos(column, row);
	Point span(column_span, row_span);

	// Newly added widget should not overlap with existing ones.
	for (auto &c : children) {
		if (check_box_collision(pos, span, c.grid_pos, c.span))
			return nullptr;
	}

	// Extend the grid if its too small for the widget.
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
	});

	return ret_ptr;
}

Widget *Grid::notify(Event ev)
{
	for (auto &c : children) {
		if (!c.widget->collides_with_point(ev.cursor))
			continue;
		return notify_widget(*c.widget, ev);
	}

	return nullptr;
}

void Grid::draw_debug()
{
	// Draw each row and column line, also draw the gap between them.
	// We draw_rect_lines because draw_line produces a dim line, IDK why.
	for (int i = 0; i < col_count; ++i) {
		Point start(col_offsets[i], 0);
		Point size(col_sizes[i], get_size().y);
		draw_rect_lines(start, size, GAP_COLOR);

		// Fill the gap
		start += Point(size.x, 0);
		if (col_gap != 0 && i != col_count - 1)
			draw_rect(start, Point(col_gap, get_size().y), GAP_FILL_COLOR);
	}

	for (int i = 0; i < row_count; ++i) {
		Point start(0, row_offsets[i]);
		Point size(get_size().x, row_sizes[i]);
		draw_rect_lines(start, size, GAP_COLOR);

		// Fill the gap
		start += Point(0, size.y);
		if (row_gap != 0 && i != row_count - 1)
			draw_rect(start, Point(get_size().x, row_gap), GAP_FILL_COLOR);
	}

	// Draw the outer box over grid lines, since lines are drawn as rectangles.
	Widget::draw_debug();

	for (auto &c : children)
		draw_widget_debug(*c.widget);
}

void Grid::draw()
{
	for (auto &c : children)
		draw_widget(*c.widget);
}

void Grid::layout_children(Point size_hint)
{
	assert(row_count == int(row_sizes.size()));
	assert(col_count == int(col_sizes.size()));

	size_hint = clamp_components(size_hint, get_min_size(), get_max_size());

	// Expand if more space is available, every expandable cell is expanded
	// along each axis proportional to its expandable size in that axis.
	// If available space is less than the minimum space then do not shrink.
	// We only expand the cells, not the gaps.
	Point gaps_size((col_count - 1) * col_gap, (row_count - 1) * row_gap);
	Point gapless_size = size_hint - gaps_size;

	calc_expanded_size(col_min_sizes, col_max_sizes, gapless_size.x, col_sizes);
	calc_expanded_size(row_min_sizes, row_max_sizes, gapless_size.y, row_sizes);

	// Calculate offsets
	Point cont_size(
		calc_box_offsets(col_sizes, col_gap, col_offsets),
		calc_box_offsets(row_sizes, row_gap, row_offsets)
	);

	// Calculate the size available for each child for alignment purposes.
	// Cell(s) may become larger than the widget due to another widget in the
	// same row/column having larger size.
	// Then stretch accordingly and layout each child.
	for (auto &[w, grid_pos, span] : children) {
		auto [start, end] = pair(grid_pos, grid_pos + span);
		end -= Point(1, 1);
		Point avail_size(
			col_offsets[end.x] - col_offsets[start.x] + col_sizes[end.x],
			row_offsets[end.y] - row_offsets[start.y] + row_sizes[end.y]
		);

		// Setting the size does layout if container
		auto size = calc_stretched_size(
			w->get_min_size(), w->get_max_size(), avail_size, w->get_fill()
		);
		w->set_size(size);

		Point pos(col_offsets[grid_pos.x], row_offsets[grid_pos.y]);
		pos += calc_align_offset(
			w->get_size(), avail_size, w->get_horiz_align(), w->get_vert_align()
		);
		w->set_position(pos);
	}

	assert(cont_size == size_hint);
	Widget::set_size(cont_size);
}

Point Grid::calc_layout_info()
{
	alloc_row_col_data();

	// We do layout calculation for inner containers but do not actually
	// layout their children, since doing that requires size available
	// for the container which we not have right now.
	for (auto &c : children)
		calc_layout_info_if_container(*c.widget);

	// Calculate minimum and maximum size of each row and column.
	// If a widget spans multiple cells then, we also need to consider the
	// gaps between each cell for calculating cell size, since it will span
	// those gaps too. For that we simply remove the amount of size it spans
	// on gaps and only use the remaining size for cells.
	for (auto &c : children) {
		auto max_size = c.widget->get_max_size();
		auto min_size = c.widget->get_min_size();

		const Point gap(col_gap, row_gap);
		auto gap_taken = mul_components(c.span - Point(1, 1), gap);
		min_size -= gap_taken;
		max_size -= gap_taken;

		int cell_w_min = min_size.x / c.span.x;
		int cell_h_min = min_size.y / c.span.y;

		int cell_w_max = max_size.x / c.span.x;
		int cell_h_max = max_size.y / c.span.y;

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

	Point min_size(
		calc_length_with_gaps(col_min_sizes, col_gap),
		calc_length_with_gaps(row_min_sizes, row_gap)
	);
	Point max_size(
		calc_length_with_gaps(col_max_sizes, col_gap),
		calc_length_with_gaps(row_max_sizes, row_gap)
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