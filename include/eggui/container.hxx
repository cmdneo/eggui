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

	virtual void layout_child_widget(Widget &child) = 0;

	/// @brief Add a widget to the container, no layout calculation is done.
	/// @param w The widget
	/// @param anchor Anchor direction
	/// @return
	virtual void add_widget(Anchor anchor, std::unique_ptr<Widget> w);

	void set_position(Point new_pos) override
	{
		auto delta = new_pos - get_position();
		Widget::set_position(new_pos);

		for (auto &c : children)
			c->set_position(c->get_position() + delta);
	}

	void draw() override
	{
		for (auto &c : children)
			c->draw();
	}

	void draw_debug() override;

	void set_size(int width, int height) override;

	Widget *get_active_widget_at(Point point) override
	{
		for (auto &c : children) {
			if (auto p = c->get_active_widget_at(point))
				return p;
		}
		return nullptr;
	}

	bool is_strechable = true;

protected:
	// Keeps track the rectangular area available while the laying out widgets.
	// Free range is start inclusive, just like left inclusive range but in 2D.
	Point free_start{};
	Point free_end{};
	std::vector<std::unique_ptr<Widget>> children;
};

/// @brief Simple container for laying out widgets.
/// If the widget is strechable then its height is changed to fill the entire
/// column in which it is placed.
class HorizontalContainer : public Container
{
public:
	using Container::Container;

	Point calc_min_size() override;
	void layout_child_widget(Widget &child) override;
};

/// @brief Simple container for laying out widgets.
/// If the widget is strechable then its width is changed to fill the entire
/// row in which it is placed.
class VerticalContainer : public Container
{
public:
	using Container::Container;

	Point calc_min_size() override;
	void layout_child_widget(Widget &child) override;
};

/// @brief Flexible container
// TODO Implement this container
class FlexContainer : public Container
{
};

} // namespace eggui

#endif
