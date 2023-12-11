#ifndef SCROLLABLE_HXX_INCLUDED
#define SCROLLABLE_HXX_INCLUDED

#include <memory>
#include <utility>
#include <functional>

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

	Point get_center_position() const
	{
		return get_position() + get_size() / 2;
	}

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;

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

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;
	void draw_debug_impl() override;

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
	);

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;
	void draw_debug_impl() override;

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