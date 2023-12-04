#ifndef CONTAINER_HXX_INCLUDED
#define CONTAINER_HXX_INCLUDED

#include <memory>
#include <utility>

#include "point.hxx"
#include "event.hxx"
#include "widget.hxx"

namespace eggui
{
enum class Alignment : char {
	Start,
	Center,
	End,
};

// Direction can be used as bit flags extracting by its underying type.
enum class Direction : unsigned {
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

	virtual void layout_children() = 0;

	void draw_debug() override;

	void draw() override {}

	void set_size(int width, int height) override;

protected:
	// Keeps track the rectangular area available while the laying out widgets.
	// Free range is start inclusive, just like left inclusive range but in 2D.
	Point free_start{};
	Point free_end{};

	std::vector<std::unique_ptr<Widget>> children;
};

// /// @brief Has a single child, used to give padding
// /// and alignement to another widgets.
// class SimpleContainer final : public Container
// {
// public:
// 	SimpleContainer(std::unique_ptr<Widget> child_)
// 		: child(std::move(child_))
// 	{
// 	}

// 	void layout_children() override {}

// private:
// 	std::unique_ptr<Widget> child;
// };

// /// @brief Simple container for laying out widgets.
// /// If the widget is strechable then its height is changed to fill the entire
// /// column in which it is placed.
// class HorizontalContainer final : public Container
// {
// public:
// 	using Container::Container;

// 	void layout_children() override {}
// 	Point calc_min_size() override;

// protected:
// 	void layout_child_widget(Widget &child);
// };

// /// @brief Simple container for laying out widgets.
// /// If the widget is strechable then its width is changed to fill the entire
// /// row in which it is placed.
// class VerticalContainer final : public Container
// {
// public:
// 	using Container::Container;

// 	void layout_children() override {}
// 	Point calc_min_size() override;

// protected:
// 	void layout_child_widget(Widget &child);
// };

class Grid final : public Container
{
public:
	using Container::Container;

	Grid() = default;

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
		std::unique_ptr<Widget> child, const Widget *beside, Direction stick,
		int column_span = 1, int row_span = 1
	);

	// TODO Return something else other than the children added??
	/// @brief Add a widget to the grid.
	/// @param child The widget
	/// @param column Start column
	/// @param row Start row
	/// @param column_span Default=1
	/// @param row_span Default=1
	/// @return Pointer to the child if added, otherwise nullptr.
	Widget *add_widget(
		std::unique_ptr<Widget> child, int column, int row, int column_span = 1,
		int row_span = 1
	);

	/// @brief Do layout calculation set position for each children in the grid.
	void layout_children() override;

	Widget *notify(Event ev) override;
	void set_position(Point new_pos) override;
	void set_size(int width, int height) override;
	void draw_debug() override;
	void draw() override;

private:
	struct Child {
		std::unique_ptr<Widget> widget;
		// Size available to the widget can be larger than its size
		// This occcurs when the cell(s) it lives in become larger due another
		// widget in the same column/row having larger size.
		Point avail_size;
		// Top left column and row
		Point grid_pos;
		// Number of columns and rows spanned
		Point span;
		Alignment h_align;
		Alignment v_align;
	};

	std::vector<Child> children;

	// Since sizes can be fractional and using integers may cause
	// them to drift by a few pixels, therefore, we use floats.
	// It is the size needed by the largest cell in row/column.
	std::vector<float> row_sizes;
	std::vector<float> col_sizes;
	// Position from where the row/column starts relative to grid position.
	std::vector<float> row_offsets;
	std::vector<float> col_offsets;

	int row_count = 0;
	int col_count = 0;
	int row_gap = 0;
	int col_gap = 0;
};

} // namespace eggui

#endif
