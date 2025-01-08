#ifndef SCROLLABLE_HXX_INCLUDED
#define SCROLLABLE_HXX_INCLUDED

#include <utility>
#include <memory>
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

	[[nodiscard]] auto get_center_pos() const -> Point
	{
		return get_position() + get_size() / 2;
	}

protected:
	auto notify(Event ev) -> Widget * override;
	void draw() override;

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

	/// @brief Set scroll fraction and notify the view about it.
	/// @param frac Scroll fraction in range [0, 1].
	void set_scroll_fraction(float frac);
	[[nodiscard]] inline auto get_scroll_fraction() const -> float { return scroll_fraction; }

	void set_slider_len(int len);

protected:
	auto notify(Event ev) -> Widget * override;
	void draw() override;
	void draw_debug() override;

private:
	std::function<void(float)> on_scroll = [](float) {};
	ScrollSlider slider;
	float scroll_fraction = 0.0;
	const Axis AXIS{};
};

class ScrollArea : public Container
{
public:
	ScrollArea(int w, int h, std::shared_ptr<Widget> child_);

	/// @brief Enable/Disable scrolling per axis.
	///
	/// If scrolling is disabled for an axis then, it will adjust its
	/// size to the child's (minimum) size along that axis.
	void set_scroll(bool scroll_in_x, bool scroll_in_y);

protected:
	auto notify(Event ev) -> Widget * override;
	void draw() override;
	void draw_debug() override;

	void layout_children(Point size_hint) override;
	auto calc_layout_info() -> Point override;

private:
	// Enable/Disable scrolling along each axis.
	bool is_xscroll = false, is_yscroll = false;
	// How much we have scrolled.
	float xscroll = 0, yscroll = 0;
	std::shared_ptr<Widget> child = nullptr;
};
} // namespace eggui

#endif