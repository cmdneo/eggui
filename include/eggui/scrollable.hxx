#ifndef SCROLLABLE_HXX_INCLUDED
#define SCROLLABLE_HXX_INCLUDED

#include <memory>
#include <utility>
#include <functional>

#include "event.hxx"
#include "point.hxx"
#include "widget.hxx"
#include "container.hxx"

namespace eggui
{

class ScrollSlider : public Interactive
{
public:
	ScrollSlider(int w, int h)
		: Interactive(w, h)
	{
	}

	void set_on_drag(std::function<void(Point delta)> callback)
	{
		on_drag = callback;
	}

	Point get_center_postion() const { return get_position() + get_size() / 2; }

	Widget *notify(Event ev) override;
	void draw() override;

private:
	std::function<void(Point delta)> on_drag;
};

class ScrollBar : public Interactive
{
public:
	enum Axis { X = 0, Y = 1 };

	ScrollBar(int w, int h, Axis axis);

	void set_on_scroll(std::function<void(float)> callback)
	{
		on_scroll = callback;
	}

	void set_scroll_position(Point rel_click_pos);

	void set_position(Point pos) override
	{
		Interactive::set_position(pos);
		slider.set_position(pos);
	}

	Widget *notify(Event ev) override;
	void draw() override;

private:
	std::function<void(float)> on_scroll;
	ScrollSlider slider;
	Axis scroll_axis;
};

class ScrollableView : Widget
{
public:
	ScrollableView(
		std::unique_ptr<Container> container_, bool x_axis, bool y_axis,
		bool invert_axes = true
	)
		: Widget(0, 0)
		, container(std::move(container_))
		, h_scrollbar(0, 0, ScrollBar::Axis::X)
		, v_scrollbar(0, 0, ScrollBar::Axis::Y)
		, x_scroll_enabled(x_axis)
		, y_scroll_enabled(y_axis)
		, axes_inverted(invert_axes)
	{
		h_scrollbar.set_parent(this);
		v_scrollbar.set_parent(this);
	}

	Widget *notify(Event ev) override;
	void draw() override;

private:
	std::unique_ptr<Container> container = nullptr;
	ScrollBar h_scrollbar;
	ScrollBar v_scrollbar;

	bool x_scroll_enabled = false;
	bool y_scroll_enabled = false;
	bool axes_inverted = true;
};
} // namespace eggui

#endif