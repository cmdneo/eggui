#ifndef CONTAINER_HXX_INCLUDED
#define CONTAINER_HXX_INCLUDED

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>
#include <typeinfo>

#include "widget.hxx"
#include "graphics.hxx"

namespace eggui
{
class Container : public Widget
{
public:
	Container(int w = 0, int h = 0)
		: Widget(0, 0, w, h)
	{
	}

	/// @brief Layout all the children using layout info and set size.
	/// @param size_hint Size available for the layout.
	/// @note `calc_layout_info` must be called before this.
	virtual void layout_children(Point size_hint) = 0;
	/// @brief Calculate layout info but do not actually layout anything.
	/// @return Minimum size needed for the container to prevent overflow.
	virtual Point calc_layout_info() = 0;

	void set_size(Point new_size) override;

protected:
	// Generally a layout calculation needed only when a child is
	// added or removed from the container or layout config is changed.
	// bool needs_layout_calc = true;
};

class PaddedBox : public Container
{
public:
	PaddedBox(std::shared_ptr<Widget> child_)
		: child(std::move(child_))
	{
	}

	/// @brief Set minimum padding for each direction, if a value is negative
	/// then the old padding for that direction is preserved.
	void set_padding(int top, int bottom, int left, int right);
	void layout_children(Point size_hint) override;
	Point calc_layout_info() override;

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;
	void draw_debug_impl() override;

private:
	std::shared_ptr<Widget> child;

	int top_pad = 0;
	int bottom_pad = 0;
	int left_pad = 0;
	int right_pad = 0;

	// Expand inner widget.
	Fill fill_mode = Fill::RowNColumn;
};

class LinearBox final : public Container
{
public:
	LinearBox(Orientation orient)
		: Container(0, 0)
		, orientation(orient)
	{
	}

	/// @brief Expand the box to fill space available along the orientation.
	/// @param can_expand Value
	void set_expand_to_fill(bool can_expand) { expand_to_fill = can_expand; }
	void set_gap(int gap) { item_gap = gap; }

	Widget *add_widget_start(std::shared_ptr<Widget> child);
	Widget *add_widget_end(std::shared_ptr<Widget> child);

	void layout_children(Point avail_size) override;
	Point calc_layout_info() override;

protected:
	Widget *notify_impl(Event ev) override;
	void draw_debug_impl() override;
	void draw_impl() override;

private:
	struct Child {
		std::shared_ptr<Widget> widget;
		Alignment align;
		Fill fill;
	};

	/// @brief Calculate gap size in both components.
	/// @return Gap size.
	Point calc_gap_size() const;

	// Children pushed to start, first element is placed at start.
	std::vector<Child> start_children;
	// Children pushed to end, first element is placed at the end.
	std::vector<Child> end_children;

	// Layout orientation: row or column.
	Orientation orientation = Orientation::Horizontal;
	// Meaning of the below depend on orientation.
	// Relative position of cells parallel to orientation.
	std::vector<int> cell_offsets;
	// Length of each cell along the orientation.
	std::vector<int> cell_sizes;
	// Minimum and maximum sizes for each cell parallel to orientation.
	std::vector<int> cell_max_sizes;
	std::vector<int> cell_min_sizes;
	// Gap between each item along the orientation.
	int item_gap = 0;

	// Expand the box in the along the orientation direction to fill the space.
	bool expand_to_fill = false;
};

class Grid final : public Container
{
public:
	using Container::Container;

	void set_row_gap(int gap) { row_gap = gap; }
	void set_col_gap(int gap) { col_gap = gap; }

	/// @brief Add widget beside another in the specified direction.
	/// @param child The widget
	/// @param beside Beside which widget will be added
	/// @param stick Sticky direction
	/// @param column_span Default=1
	/// @param row_span Default=1
	/// @return Pointer to the child if added, otherwise nullptr.
	Widget *add_widget_beside(
		std::shared_ptr<Widget> child, const Widget *beside, Direction stick,
		int column_span = 1, int row_span = 1
	);

	// TODO Return something else other than the child added??
	/// @brief Add a widget to the grid.
	/// @param child The widget
	/// @param column Start column
	/// @param row Start row
	/// @param column_span Default=1
	/// @param row_span Default=1
	/// @return Pointer to the child if added, otherwise nullptr.
	Widget *add_widget(
		std::shared_ptr<Widget> child, int column, int row, int column_span = 1,
		int row_span = 1
	);

	void layout_children(Point avail_size) override;
	Point calc_layout_info() override;

protected:
	Widget *notify_impl(Event ev) override;
	void draw_debug_impl() override;
	void draw_impl() override;

private:
	void alloc_row_col_data();

	struct Child {
		std::shared_ptr<Widget> widget;
		// Top left position: column and row.
		Point grid_pos;
		// Number of columns and rows spanned.
		Point span;
};

	std::vector<Child> children;

	// The size needed by the largest cell in row/column.
	std::vector<int> row_sizes;
	std::vector<int> col_sizes;
	// Grid relative position from where the row/column starts.
	std::vector<int> row_offsets;
	std::vector<int> col_offsets;
	// Maximum and minimum sizes for scaling purposes
	std::vector<int> row_max_sizes;
	std::vector<int> col_max_sizes;
	std::vector<int> row_min_sizes;
	std::vector<int> col_min_sizes;

	int row_count = 0;
	int col_count = 0;
	int row_gap = 0;
	int col_gap = 0;
};

inline void calc_layout_info_if_container(Widget &w)
{
	auto cont = dynamic_cast<Container *>(&w);
	if (cont)
		cont->calc_layout_info();
}

} // namespace eggui

#endif
