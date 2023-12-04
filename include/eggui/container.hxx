#ifndef CONTAINER_HXX_INCLUDED
#define CONTAINER_HXX_INCLUDED

#include "point.hxx"
#include "event.hxx"
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

	/// @brief Add a widget to the container, no layout calculation is done.
	/// @param w The widget
	/// @param anchor Anchor direction
	/// @return
	virtual void add_widget(Anchor anchor, std::unique_ptr<Widget> w);

	void set_position(Point new_pos) override
	{
		auto delta = new_pos - get_position();
		Widget::set_position(new_pos);

		// Moving a container should also move all of its children with it.
		for (auto &c : children)
			c->set_position(c->get_position() + delta);
	}

	Widget *notify(Event ev) override
	{
		// Handle mouse events only for now
		for (auto &c : children) {
			if (!c->collides_with_point(ev.cursor))
				continue;
			if (c->notify(ev))
				return c.get();
		}

		return nullptr;
	}

	void draw() override
	{
		for (auto &c : children) {
			// if (c->is_visible(100, 100))
			c->draw();
		}
	}

	void draw_debug() override;

	void set_size(int width, int height) override;

protected:
	virtual void layout_child_widget(Widget &child) = 0;

	// Keeps track the rectangular area available while the laying out widgets.
	// Free range is start inclusive, just like left inclusive range but in 2D.
	Point free_start{};
	Point free_end{};
	std::vector<std::unique_ptr<Widget>> children;
};

/// @brief Simple container for laying out widgets.
/// If the widget is strechable then its height is changed to fill the entire
/// column in which it is placed.
class HorizontalContainer final : public Container
{
public:
	using Container::Container;

	Point calc_min_size() override;

protected:
	void layout_child_widget(Widget &child) override;
};

/// @brief Simple container for laying out widgets.
/// If the widget is strechable then its width is changed to fill the entire
/// row in which it is placed.
class VerticalContainer final : public Container
{
public:
	using Container::Container;

	Point calc_min_size() override;

protected:
	void layout_child_widget(Widget &child) override;
};

/// @brief Flexible container
// TODO Implement this container
class FlexContainer : public Container
{
};

} // namespace eggui

#endif
