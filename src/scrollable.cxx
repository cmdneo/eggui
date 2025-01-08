#include <cassert>
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "widget.hxx"
#include "container.hxx"
#include "scrollable.hxx"
#include "theme.hxx"
#include "graphics.hxx"
#include "calc.hxx"

using namespace eggui;

// TODO Fix this broken stuff.

/// @brief Calculates slider position and scroll fraction for a scroll bar.
/// @return Scroll fraction in range [0, 1]
auto calc_scroll_fraction(int bar_len, int slider_len, int click_pos) -> float
{
	float start = slider_len / 2.;
	float end = bar_len - slider_len / 2.;
	float pos = std::clamp(float(click_pos), start, end);

	if (auto space = end - start; space > 0) {
		return (pos - start) / space;
	}
	return 0;
}

/// @brief Calculate slider center position for the scroll fraction.
/// @return Slider center position.
auto calc_slider_pos(int bar_len, int slider_len, float scroll_frac) -> int
{
	assert(0 <= scroll_frac && scroll_frac <= 1);
	return slider_len / 2. + (bar_len - slider_len) * scroll_frac;
}

/// @brief Calculates the container start position relative to scroll view.
/// @return container position relative to the view, is awlays <= 0.
auto calc_container_pos(int view_len, int cont_len, float scroll_frac) -> int
{
	int overflow = cont_len - view_len;
	return std::min(.0f, -overflow * scroll_frac);
}

/// @brief Calculates scrolled fraction for a container in a view.
/// @return Scrolled fraction in range [0, 1].
auto calc_scroll_frac(int view_len, int cont_len, int cont_pos) -> float
{
	assert(cont_pos <= 0);

	float overflow = cont_len - view_len;
	return std::max(.0f, -cont_pos / overflow);
}

/// @brief Calculate slider length for the amount of scroll needed.
/// @return Slider length
auto calc_slider_len(int view_len, int cont_len) -> int
{
	// Slider length should decrease with increase in overflow,
	// stopping at a minimum value.
	auto overflow = std::max(0, cont_len - view_len);
	auto len = view_len - overflow;
	return std::max(MIN_SLIDER_LENGTH, len);
}

// TODO Make it reveal or hide area based on last scroll direction,
//      it is not a functional but rather a UX design issue.
/// @brief Calculate the new container position as the view grows/shrinks.
/// @return New container position in scrollview.
auto calc_new_container_pos(
	int old_view_len, int new_view_len, int cont_len, int old_cont_pos
) -> int
{
	assert(old_cont_pos <= 0);

	if (new_view_len >= cont_len) {
		return 0;
	}
	if (new_view_len <= old_view_len) {
		return old_cont_pos;
	}

	int bottom_ovf = old_cont_pos + cont_len - old_view_len;
	int delta = new_view_len - old_view_len;

	delta -= std::min(delta, bottom_ovf);
	return old_cont_pos + delta;
}

// ScrollSlider members
//---------------------------------------------------------
auto ScrollSlider::notify(Event ev) -> Widget *
{
	if (handle_mouse_hover_events(ev)) {
		return this;
	}
	if (handle_mouse_press_events(ev)) {
		return this;
	}

	// Notify its parent(which is scrollbar) to do the scrolling work on drag.
	if (ev.type == EventType::MouseDrag) {
		on_drag(ev.delta);
		return this;
	}

	return nullptr;
}

void ScrollSlider::draw()
{
	auto color = BUTTON_COLOR;
	if (is_hovering) {
		color = BUTTON_HOVER_COLOR;
	}
	if (is_pressed) {
		color = BUTTON_CLICK_COLOR;
	}

	draw_rect(Point(), get_size(), color);
}

// ScrollBar members
//---------------------------------------------------------
ScrollBar::ScrollBar(int w, int h, Axis axis)
	: Interactive(w, h)
	, slider(0, 0)
	, AXIS(axis)
{
	// Slider size is set depending upon its orientation(X or Y).
	Point size;
	size[AXIS] = MIN_SLIDER_LENGTH;
	size[1 - AXIS] = SCROLL_BAR_WIDTH;
	slider.set_all_sizes(size);
	slider.set_parent(this);

	slider.set_on_drag([this](Point delta) {
		// Scroll according to slider drag by user.
		auto new_pos = slider.get_center_pos() + delta;
		const auto frac = calc_scroll_fraction(
			get_size()[AXIS], slider.get_size()[AXIS], new_pos[AXIS]
		);

		set_scroll_fraction(frac);
	});
}

auto ScrollBar::notify(Event ev) -> Widget *
{
	if (slider.collides_with_point(ev.cursor)) {
		return notify_widget(slider, ev);
	}

	if (handle_mouse_hover_events(ev)) {
		return this;
	}

	if (ev.type != EventType::MousePressed) {
		return nullptr;
	}

	// If pressed anywhere on the bar but not on the slider then,
	// we pretend that the slider was dragged to that position.
	auto pos = slider.get_center_pos();
	Event dragged(ev.window, EventType::MouseDrag, pos);
	dragged.delta = ev.cursor - pos;

	// We tell slider that it was hovered over, pressed and then dragged.
	Event tmp(ev.window, EventType::MouseIn, pos);
	notify_widget(slider, tmp);
	tmp.type = EventType::MousePressed;
	notify_widget(slider, tmp);

	return notify_widget(slider, dragged);
}

void ScrollBar::draw_debug()
{
	Widget::draw_debug();
	draw_widget_debug(slider);
}

void ScrollBar::draw()
{
	draw_rect(Point(), get_size(), SCROLL_BAR_COLOR);
	draw_widget(slider);
}

void ScrollBar::set_scroll_fraction(float frac)
{
	assert(0 <= frac && frac <= 1);

	int pos = calc_slider_pos(get_size()[AXIS], slider.get_size()[AXIS], frac);
	pos -= slider.get_size()[AXIS] / 2.;

	if (AXIS == Axis::X) {
		slider.set_xpos(pos);
	} else {
		slider.set_ypos(pos);
	}

	scroll_fraction = frac;
	on_scroll(scroll_fraction);
}

void ScrollBar::set_slider_len(int len)
{
	auto size = slider.get_size();
	size[AXIS] = len;
	slider.set_all_sizes(size);
}

// ScrollArea members
//---------------------------------------------------------
ScrollArea::ScrollArea(int w, int h, std::shared_ptr<Widget> child_)
	: Container(w, h)
	, child(std::move(child_))
{
	child->set_parent(this);
}

void ScrollArea::set_scroll(bool scroll_in_x, bool scroll_in_y)
{
	is_xscroll = scroll_in_x;
	is_yscroll = scroll_in_y;
}

auto ScrollArea::notify(Event ev) -> Widget *
{
	auto inner = notify_widget(*child, ev);

	// Find the innermost scrollable widget, if none exists then,
	// this is the one. Scrollbale means responds to scroll event.
	if (inner == nullptr && ev.type == EventType::Scroll) {
		// Scroll gives the direction content should go [negative is up].
		// TODO Implement scroll acceleration. In the window module prefereably.
		Point excess = child->get_size() - get_size();
		// Do not scroll if overflow is absent or scrolling is disabled.
		if (excess.x <= 0 || !is_xscroll) {
			ev.scroll.x = 0;
		}
		if (excess.y <= 0 || !is_yscroll) {
			ev.scroll.y = 0;
		}

		Point pos = child->get_position() + SCROLL_FACTOR * ev.scroll;
		pos = clamp_components(pos, -excess, Point(0, 0));
		child->set_position(pos);

		xscroll = std::clamp(-1.0 * pos.x / excess.x, 0.0, 1.0);
		yscroll = std::clamp(-1.0 * pos.y / excess.y, 0.0, 1.0);
		return this;
	}

	return inner;
}

void ScrollArea::draw() { draw_widget(*child); }

void ScrollArea::draw_debug()
{
	Widget::draw_debug();
	draw_widget_debug(*child);
}

void ScrollArea::layout_children(Point size_hint)
{
	// As it is a scroll-area its size has no effect on its child's size.
	// Preserves child size or layouts if child is a container to its min size.
	child->set_size(child->get_min_size());
	Widget::set_size(get_min_size());
}

auto ScrollArea::calc_layout_info() -> Point
{
	calc_layout_info_if_container(*child);

	// As it is a scroll-area its child has no effect on its size.
	// If scrolling is disabled along an axis then fit to the child's size
	// in that axis by stretching or contracting.
	auto min_sz = child->get_min_size();
	auto size = get_size();
	if (!is_xscroll) {
		size.x = min_sz.x;
	}
	if (!is_yscroll) {
		size.y = min_sz.y;
	}

	// TODO Add dynamic sizing for scrolling direction, infinite maybe.
	set_max_size(size);
	set_min_size(size);
	return size;
}
