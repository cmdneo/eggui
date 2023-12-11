#include <algorithm>
#include <utility>

#include "widget.hxx"
#include "scrollable.hxx"
#include "theme.hxx"
#include "graphics.hxx"

using namespace eggui;

/// @brief Calculates slider position and scroll fraction for a scroll bar.
/// @param bar_len    Length of the bar
/// @param slider_len Length of the slider
/// @param click_pos Position where scroll bar was clicked on
/// @return Slider-center position in range [slider_len/2, bar_len-slider_len/2]
///         and scroll fraction in range [0, 1]
std::pair<int, float>
calc_scroll_amount(int bar_len, int slider_len, int click_pos)
{
	float start = slider_len / 2.;
	float end = bar_len - slider_len / 2.;
	float pos = std::clamp(float(click_pos), start, end);

	return {pos, (pos - start) / (end - start)};
}

// ScrollSlider members
//---------------------------------------------------------
Widget *ScrollSlider::notify_impl(Event ev)
{
	if (handle_mouse_hover_events(ev))
		return this;
	if (handle_mouse_press_events(ev))
		return this;

	// We do not move it on drag, it is managed by the scrollbar it is in.
	if (ev.type == EventType::MouseDrag) {
		on_drag(ev.delta);
		return this;
	}

	return Interactive::notify_impl(ev);
}

void ScrollSlider::draw_impl()
{
	auto color = BUTTON_COLOR;
	if (is_hovering)
		color = BUTTON_HOVER_COLOR;
	if (is_pressed)
		color = BUTTON_CLICK_COLOR;

	draw_rect(Point(), get_size(), color);
}

// ScrollBar members
//---------------------------------------------------------
ScrollBar::ScrollBar(int w, int h, Axis axis)
	: Interactive(w, h)
	, slider(0, 0)
	, scroll_axis(axis)
{
	Point size[] = {Point(SLIDER_LENGTH, h), Point(w, SLIDER_LENGTH)};
	slider.set_size(size[scroll_axis]);
	slider.set_parent(this);

	slider.set_on_drag([this](Point delta) {
		auto click_pos = slider.get_center_position() + delta;
		set_scroll_position(click_pos);
	});
}

Widget *ScrollBar::notify_impl(Event ev)
{
	if (slider.collides_with_point(ev.cursor))
		return slider.notify(ev);

	if (handle_mouse_hover_events(ev))
		return this;

	// If pressed anywhere on the bar but not on the slider then,
	// we pretend that the slider was dragged to that position.
	if (ev.type != EventType::MousePressed)
		return Interactive::notify_impl(ev);

	// We tell slider that it was hovered, pressed then dragged.
	auto pos = slider.get_center_position();
	slider.notify(Event(EventType::MouseIn, pos));
	slider.notify(Event(EventType::MousePressed, pos));

	Event dragged(EventType::MouseDrag, pos);
	dragged.delta = ev.cursor - pos;
	return slider.notify(dragged);
}

void ScrollBar::draw_debug_impl()
{
	Widget::draw_debug_impl();
	slider.draw_debug();
}

void ScrollBar::draw_impl()
{
	draw_rect(Point(), get_size(), SCROLL_BAR_COLOR);
	slider.draw();
}

void ScrollBar::set_scroll_position(Point relative_click_pos)
{
	const auto [center, fract] = calc_scroll_amount(
		get_size()[scroll_axis], slider.get_size()[scroll_axis],
		relative_click_pos[scroll_axis]
	);

	// Set slider postion and tell ScrollView about the scroll
	int start = center - slider.get_size()[scroll_axis] / 2;
	Point offset_xy[] = {Point(start, 0), Point(0, start)};
	slider.set_position(offset_xy[scroll_axis]);
	on_scroll(fract);
}

// ScrollableView members
//---------------------------------------------------------
ScrollableView::ScrollableView(
	std::unique_ptr<Container> container_, bool x_axis, bool y_axis,
	bool invert_axes
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

Widget *ScrollableView::notify_impl(Event ev)
{
	// Find the innermost scrollable widget, if none exists then,
	// this is the one. Scrollbale means responds to scroll event.
	if (ev.type == EventType::Scroll) {
		auto inner = container->notify(ev);
		return inner ? inner : this;
	}

	if (h_scrollbar.collides_with_point(ev.cursor))
		return h_scrollbar.notify(ev);
	if (v_scrollbar.collides_with_point(ev.cursor))
		return v_scrollbar.notify(ev);

	return container->notify(ev);
}

void ScrollableView::draw_debug_impl()
{
	Widget::draw_debug_impl();
	v_scrollbar.draw_debug();
}

void ScrollableView::draw_impl()
{
	// Scrollable does not have its own texture
}