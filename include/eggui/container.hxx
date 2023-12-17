#ifndef CONTAINER_HXX_INCLUDED
#define CONTAINER_HXX_INCLUDED

#include <cstdint>
#include <memory>
#include <vector>
#include <deque>
#include <utility>

#include "widget.hxx"
#include "graphics.hxx"

namespace eggui
{
enum class Alignment {
	Start,
	Center,
	End,
};

enum class Orientation {
	Horizontal,
	Vertical,
};

enum class Fill : std::uint8_t {
	None,
	StretchRow,
	StretchColumn,
	Stretch,
};

// Direction can be used as bit flags extracting by its underying integer.
enum class Direction : std::uint8_t {
	Top = 1,
	Bottom = 1 << 1,
	Left = 1 << 2,
	Right = 1 << 3,
	TopLeft = Top | Left,
	TopRight = Top | Right,
	BottomLeft = Bottom | Left,
	BottomRight = Bottom | Right,
};

class Container : public Widget
{
public:
	Container()
		: Widget(0, 0, 0, 0)
	{
	}

	/// @brief Layout all the children using layout info and set size.
	/// @param size_hint Size available for the layout.
	/// @note `calc_layout_info` must be called before this.
	virtual void layout_children(Point size_hint) = 0;
	/// @brief Calculate layout info but do not actually layout anything.
	/// @return Minimum size needed for the container to prevent overflow.
	/// @note A layout calculation is performed only when required,
	///       that is, when at least one child has been added or removed.
	virtual Point calc_layout_info() = 0;

	void set_size(Point new_size) override;

protected:
	// Generally a layout calculation needed only when a child is
	// added or removed from the container.
	bool needs_layout_calc = true;
};

class LinearBox final : public Container
{
public:
	using Container::Container;

	LinearBox(Orientation orient)
		: orientation(orient)
	{
	}

	void set_gap(int gap) { item_gap = gap; }

	Widget *add_widget_start(std::unique_ptr<Widget> child);
	Widget *add_widget_end(std::unique_ptr<Widget> child);

	void layout_children(Point avail_size) override;
	Point calc_layout_info() override {}

protected:
	Widget *notify_impl(Event ev) override;
	void set_position(Point new_pos) override;
	void draw_debug_impl() override;
	void draw_impl() override;

private:
	/// @brief Get x or y of a point depending on the orientation.
	/// @param pt
	/// @return
	// inline int get_ocoord(Point pt) {}

	// Deque because we will need to add and remove elements from both ends.
	// Children pushed to start, first element is placed at the start.
	std::deque<std::unique_ptr<Widget>> start_children;
	// Children pushed to end, last element is placed at the end.
	std::deque<std::unique_ptr<Widget>> end_children;

	// Layout orientation: row or column.
	Orientation orientation = Orientation::Horizontal;
	// Meaning of the below three depend on orientation.
	// Box relative position parallel to orientation from where the cell starts.
	std::vector<float> cell_offsets;
	// Size of each cell in the parallel to orientation.
	std::vector<float> cell_sizes;
	// Size of every cell perpendicular to orientation.
	int max_cell_size;

	int item_gap = 0;
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
	/// @param fill Enable/disable stretch to fill, default=None.
	/// @return Pointer to the child if added, otherwise nullptr.
	Widget *add_widget_beside(
		std::unique_ptr<Widget> child, const Widget *beside, Direction stick,
		int column_span = 1, int row_span = 1, Fill fill = Fill::None
	);

	// TODO Return something else other than the child added??
	/// @brief Add a widget to the grid.
	/// @param child The widget
	/// @param column Start column
	/// @param row Start row
	/// @param column_span Default=1
	/// @param row_span Default=1
	/// @param fill Enable/disable stretch to fill, default=None.
	/// @return Pointer to the child if added, otherwise nullptr.
	Widget *add_widget(
		std::unique_ptr<Widget> child, int column, int row, int column_span = 1,
		int row_span = 1, Fill fill = Fill::None
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
		std::unique_ptr<Widget> widget;
		// Top left column and row
		Point grid_pos;
		// Number of columns and rows spanned
		Point span;
		Alignment h_align;
		Alignment v_align;
		Fill fill;
	};

	std::vector<Child> children;

	// The size needed by the largest cell in row/column.
	std::vector<float> row_sizes;
	std::vector<float> col_sizes;
	// Grid relative position from where the row/column starts.
	std::vector<float> row_offsets;
	std::vector<float> col_offsets;
	// Maximum and minimum sizes for scaling purposes
	std::vector<float> row_max_sizes;
	std::vector<float> col_max_sizes;
	std::vector<float> row_min_sizes;
	std::vector<float> col_min_sizes;

	int row_count = 0;
	int col_count = 0;
	int row_gap = 0;
	int col_gap = 0;
};

} // namespace eggui

#endif
