#include <cassert>
#include <algorithm>
#include <functional>
#include <optional>
#include <utility>

#include "widget.hxx"
#include "container.hxx"
#include "scrollable.hxx"
#include "theme.hxx"
#include "graphics.hxx"
#include "calc.hxx"

using namespace eggui;

using std::reference_wrapper;

/// @brief Calculates slider position and scroll fraction for a scroll bar.
/// @param bar_len Length of the scrollbar.
/// @param slider_len Length of the slider.
/// @param click_pos Position where scroll bar was clicked on.
/// @return Scroll fraction in range [0, 1]
float calc_scroll_fraction(int bar_len, int slider_len, int click_pos)
{
	float start = slider_len / 2.;
	float end = bar_len - slider_len / 2.;
	float pos = std::clamp(float(click_pos), start, end);

	if (auto space = end - start; space > POINT_EPSILON)
		return (pos - start) / space;
	return 0;
}

/// @brief Calculate slider center position for the scroll fraction.
/// @param bar_len Length of the scrollbar.
/// @param slider_len Length of the slider.
/// @param scroll_frac Slider scroll fraction.
/// @return Slider center position.
int calc_slider_pos(int bar_len, int slider_len, float scroll_frac)
{
	assert(0 <= scroll_frac && scroll_frac <= 1);
	return slider_len / 2. + (bar_len - slider_len) * scroll_frac;
}

/// @brief Calculates the container start position relative to scroll view.
/// @param view_len Length of the view.
/// @param cont_len Length of the container.
/// @param scroll_frac Scroll amount.
/// @return container position relative to the view, is awlays <= 0.
int calc_container_pos(int view_len, int cont_len, float scroll_frac)
{
	int overflow = cont_len - view_len;
	return std::min(.0f, -overflow * scroll_frac);
}

/// @brief Calculates scrolled fraction for a container in a view.
/// @param view_len View length.
/// @param cont_len Container length.
/// @param cont_pos Container position relative to the view.
/// @return Scrolled fraction in range [0, 1].
float calc_scroll_frac(int view_len, int cont_len, int cont_pos)
{
	assert(cont_pos <= 0);

	float overflow = cont_len - view_len;
	return std::max(.0f, -cont_pos / overflow);
}

/// @brief Calculate slider length for the amount of scroll needed.
/// @param view_len Lenght of the scroll view visible, it also bar length.
/// @param cont_len Container length.
/// @return Slider length
/// @note
int calc_slider_len(int view_len, int cont_len)
{
	// Slider length should decrease with increase in overflow,
	// stopping at a minimum value.
	auto overflow = std::max(0, cont_len - view_len);
	auto len = view_len - overflow;
	return std::max(MIN_SLIDER_LENGTH, len);
}

/// @brief Calculate the new container position as the view grows/shrinks.
/// @param old_view_len Old view length.
/// @param new_view_len New view length.
/// @param cont_len Container lenght.
/// @param old_cont_pos Container position in the old view.
/// @return New container position.
int calc_new_container_pos(
	int old_view_len, int new_view_len, int cont_len, int old_cont_pos
)
{
	assert(old_cont_pos <= 0);

	int top_ovf = -old_cont_pos;
	int bottom_ovf = old_cont_pos + cont_len - old_view_len;

	// TODO calc_new_container_pos
	// For now we reveal/hide from the bottom,
	// if content hidden at the bottom is over then we reveal from the top.
	int delta = new_view_len - old_view_len;
	if (bottom_ovf < 0) {
	}

	return 0;
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
	Point size;
	size[scroll_axis] = MIN_SLIDER_LENGTH;
	size[1 - scroll_axis] = SCROLL_BAR_WIDTH;

	slider.set_parent(this);
	slider.set_all_sizes(size);

	slider.set_on_drag([this](Point delta) {
		auto click_pos = slider.get_center_position() + delta;
		scroll_to_position(click_pos);
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

	auto pos = slider.get_center_position();
	Event dragged(ev.window, EventType::MouseDrag, pos);
	dragged.delta = ev.cursor - pos;

	// We tell slider that it was hovered, pressed and then dragged,
	// for pretending that the slider was dragged to that position.
	slider.notify(Event(ev.window, EventType::MouseIn, pos));
	slider.notify(Event(ev.window, EventType::MousePressed, pos));
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

void ScrollBar::scroll_to_position(Point bar_click_pos)
{
	const auto frac = calc_scroll_fraction(
		get_size()[scroll_axis], slider.get_size()[scroll_axis],
		bar_click_pos[scroll_axis]
	);

	set_scroll_fraction(frac);
}

void ScrollBar::set_scroll_fraction(float frac)
{
	assert(0 <= frac && frac <= 1);

	int pos = calc_slider_pos(
		get_size()[scroll_axis], slider.get_size()[scroll_axis], frac
	);
	pos -= slider.get_size()[scroll_axis] / 2.;

	Point offsets[] = {Point(pos, 0), Point(0, pos)};
	slider.set_position(offsets[scroll_axis]);

	scroll_fraction = frac;
	on_scroll(scroll_fraction);
}

void ScrollBar::set_slider_len(int len)
{
	auto size = slider.get_size();
	size[scroll_axis] = len;
	slider.set_all_sizes(size);
}

// VScrollView members
//---------------------------------------------------------
VScrollView::VScrollView(std::unique_ptr<Widget> child_, int w, int h)
	: Container(w, h)
	, child(std::move(child_))
{
	child->set_parent(this);

	scrollbar.emplace(0, 0, AXIS);
	scrollbar->set_parent(this);

	scrollbar->set_on_scroll([this](float frac) {
		auto view_len = get_size()[AXIS];
		auto cont_len = child->get_size()[AXIS];
		auto pos = child->get_position();

		pos[AXIS] = calc_container_pos(view_len, cont_len, frac);
		child->set_position(pos);
	});
}

void VScrollView::layout_children(Point size_hint)
{
	size_hint = clamp_components(size_hint, get_min_size(), get_max_size());

	// For vertical scroll the available height provided to the child is its
	// preferred height, since the height is unbounded in vertical scroll.
	// But the available width, it is width available minus bar width.
	auto avail_size = size_hint - calc_bars_size();
	avail_size[AXIS] = child->get_preffered_size()[AXIS];

	auto child_size = calc_stretched_size(
		child->get_min_size(), child->get_max_size(), avail_size,
		Fill::RowNColumn
	);
	int child_pos = calc_new_container_pos(
		get_size()[AXIS], size_hint[AXIS], child_size[AXIS],
		child->get_position()[AXIS]
	);

	child->set_size(child_size);
	child->set_ypos(child_pos);

	auto frac = calc_scroll_frac(
		get_size()[AXIS], child->get_size()[AXIS], child->get_position()[AXIS]
	);
	scrollbar->set_scroll_fraction(frac);
	scrollbar->set_size(Point(SCROLL_BAR_WIDTH, size_hint.y));
	scrollbar->set_position(Point(size_hint.x - SCROLL_BAR_WIDTH, 0));

	// Set slider length depending on how much content is overflowing.
	// Like if oveflow is less then, less scroll is needed, so we increase
	// slide length to decrease its scroll space and vice-versa.
	auto len = calc_slider_len(size_hint[AXIS], child->get_size()[AXIS]);
	scrollbar->set_slider_len(len);

	Widget::set_size(size_hint);
}

Point VScrollView::calc_layout_info()

{
	calc_layout_info_if_container(*child);

	Point min_size = child->get_min_size() + calc_bars_size();
	Point max_size = child->get_max_size() + calc_bars_size();

	// Height is unbounded in a vertical scroll view, therefore, for maximum
	// we put no limit, and for minimum we preserve what was given.
	min_size[AXIS] = get_min_size()[AXIS];
	max_size[AXIS] = UNLIMITED_MAX_SIZE[AXIS];
	set_min_size(min_size);
	set_max_size(max_size);

	scrollbar->set_min_size(Point(SCROLL_BAR_WIDTH, min_size[AXIS]));
	scrollbar->set_max_size(Point(SCROLL_BAR_WIDTH, max_size[AXIS]));

	return get_min_size();
}

Widget *VScrollView::notify_impl(Event ev)
{
	// Find the innermost scrollable widget, if none exists then,
	// this is the one. Scrollbale means responds to scroll event.
	if (ev.type == EventType::Scroll) {
		auto inner = child->notify(ev);
		if (inner)
			return inner;

		// Scroll gives the direction content should go, and if content goes
		// up(-ve direction), it increases scroll fraction and vice-versa.
		auto scroll_frac = -ev.scroll[AXIS] / child->get_size()[AXIS];
		scroll_frac *= SCROLL_FACTOR;
		scroll_frac += scrollbar->get_scroll_fraction();
		scroll_frac = std::clamp(scroll_frac, 0.f, 1.f);
		scrollbar->set_scroll_fraction(scroll_frac);

		return this;
	}

	if (scrollbar->collides_with_point(ev.cursor))
		return scrollbar->notify(ev);

	return child->notify(ev);
}

void VScrollView::draw_impl()
{
	child->draw();
	scrollbar->draw();
}

void VScrollView::draw_debug_impl()
{
	Widget::draw_debug_impl();
	scrollbar->draw_debug();

	child->draw_debug();
}

Point VScrollView::calc_bars_size() const { return Point(SCROLL_BAR_WIDTH, 0); }
