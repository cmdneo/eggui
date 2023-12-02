#ifndef CONTAINER_HXX_INCLUDED
#define CONTAINER_HXX_INCLUDED

#include "point.hxx"
#include "widget.hxx"

namespace eggui
{
class Container : public Widget
{
public:
	Container()
		: Widget(0, 0, 0, 0)
	{
	}

	virtual void layout_children() = 0;

	/// @brief Add a widget to the container, no layout calculation is done.
	/// @param w The widget
	/// @param anchor Anchor direction
	/// @return
	virtual bool add_widget(Anchor anchor, std::unique_ptr<Widget> w);

	/// @brief Update size and calculate the new re-layout its children(if any)
	///       according to its new size.
	/// @param width  New width
	/// @param height New height
	void set_size(int width, int height);

	Point get_min_size() const { return packed_size; }

	void draw() override
	{
		for (auto &c : children)
			c->draw();
	}

	void draw_debug() override;

protected:
	// Keeps track the rectangular area available.
	// Free range is start inclusive, just like left inclusive range but in 2D.
	Point free_start{};
	Point free_end{};
	// Size of the container when all the widgets are packed together.
	// This generally indicates the minimum size the container should be of.
	// TODO implement this calculation.
	Point packed_size{};
};

/// @brief Simple container for laying out widgets.
/// If a widget is placed then it the entire columns
/// in which it is placed is mark filled.
class HorizontalContainer : public Container
{
public:
	using Container::Container;

	void layout_children() override;
};

/// @brief Simple container for laying out widgets.
/// If a widget is placed then it the entire rows
/// in which it is placed is mark filled.
class VerticalContainer : public Container
{
public:
	using Container::Container;

	void layout_children() override;
};

} // namespace eggui

#endif
