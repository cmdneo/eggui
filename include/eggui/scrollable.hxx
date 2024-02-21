#ifndef SCROLLABLE_HXX_INCLUDED
#define SCROLLABLE_HXX_INCLUDED

#include <memory>
#include <utility>
#include <optional>
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
	std::function<void(Point delta)> on_drag = [](Point) {};
};

class ScrollBar : public Interactive
{
public:
	enum Axis { X = 0, Y = 1 };
	static constexpr Axis AXES[]{Axis::X, Axis::Y};

	ScrollBar(int w, int h, Axis axis);

	void set_on_scroll(std::function<void(float frac)> callback)
	{
		on_scroll = callback;
	}

	/// @brief Scroll to position on scrollbar.
	/// @param bar_click_pos Position on the bar where the slider should be centered.
	void scroll_to_position(Point bar_click_pos);

	/// @brief Set scroll fraction and notify the view about it.
	/// @param frac Scroll fraction in range [0, 1].
	void set_scroll_fraction(float frac);
	inline float get_scroll_fraction() const { return scroll_fraction; }

	void set_slider_len(int len);

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;
	void draw_debug_impl() override;

private:
	std::function<void(float)> on_scroll = [](float) {};
	ScrollSlider slider;
	float scroll_fraction = 0.0;
	Axis scroll_axis;
};

class VScrollView : public Container
{
public:
	VScrollView(std::unique_ptr<Widget> child_, int w, int h);

	void layout_children(Point size_hint) override;
	Point calc_layout_info() override;

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;
	void draw_debug_impl() override;

private:
	/// @brief Size occupied by the scrollbars for both axes
	Point calc_bars_size() const;

	std::unique_ptr<Widget> child = nullptr;
	std::optional<ScrollBar> scrollbar{};

	static constexpr ScrollBar::Axis AXIS = ScrollBar::Axis::Y;
};
} // namespace eggui

#endif