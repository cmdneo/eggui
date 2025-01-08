#include <algorithm>
#include <string>
#include <memory>
#include <utility>

#include "window.hxx"
#include "theme.hxx"
#include "toast.hxx"
#include "label.hxx"
#include "graphics.hxx"

using namespace eggui;

constexpr int SLIDE_TICKS = TICKS_PER_SECOND / 4;

/// @brief Calculate toast position and slide offset for slide animation.
/// @return Position and slide offset.
static auto calc_toast_pos_n_offset(Point size, Direction corner)
	-> std::pair<Point, Point>
{
	auto has_dir = [=](Direction one) { return has_component(corner, one); };
	Point end_pos = get_window_size() - size;
	Point pos = end_pos / 2;
	Point slide(0, 0);

	auto calc_coord = [&](Direction start, Direction end, int axis) {
		int s = size[axis] + TOAST_MARGIN;
		if (has_dir(start)) {
			pos[axis] = TOAST_MARGIN;
			slide[axis] = -s;
		} else if (has_dir(end)) {
			pos[axis] = end_pos[axis] - TOAST_MARGIN;
			slide[axis] = s;
		}
	};

	calc_coord(Direction::Left, Direction::Right, 0);
	calc_coord(Direction::Top, Direction::Bottom, 1);

	// Prefer to slide top/bottom if both axes available for sliding.
	if (slide.x != 0 && slide.y != 0) {
		slide.x = 0;
	}

	return {pos, slide};
}

Toast::Toast(int w, int h, float show_time_, std::string msg, Direction corner_)
	: Widget(w, h)
	, message(w, h - TIMER_BAR_THICKNESS, msg, TEXT_COLOR)
	, corner(corner_)
	, show_time(show_time_)
{
	message.set_text_align(Alignment::Center, Alignment::Center);
}

void Toast::show_toast(Window &window)
{
	auto copied = std::make_shared<Toast>(*this);
	auto [pos, slide_offset] = calc_toast_pos_n_offset(get_size(), corner);

	pos += slide_offset;
	copied->set_position(pos);

	// Slide in, wait for show_time and then slide out.
	auto anim_slide_in = [=](int, float p) {
		copied->set_position(pos - slide_offset * p);
		return true;
	};
	auto anim_wait = [=](int, float p) {
		copied->set_timer_bar_spent(p);
		return true;
	};
	auto anim_slide_out = [=, &window](int t, float p) {
		copied->set_position(pos - slide_offset * (1 - p));
		if (t == SLIDE_TICKS) {
			window.remove_overlay(copied.get());
		}
		return true;
	};

	int wait = 0;
	window.add_animation(
		copied.get(), Animation(SLIDE_TICKS, wait, false, anim_slide_in)
	);
	wait += SLIDE_TICKS;

	window.add_animation(
		copied.get(),
		Animation(show_time * TICKS_PER_SECOND, wait, false, anim_wait)
	);
	wait += show_time * TICKS_PER_SECOND;

	window.add_animation(
		copied.get(), Animation(SLIDE_TICKS, wait, false, anim_slide_out)
	);

	window.add_overlay(copied, 2);
}

void Toast::set_timer_bar_spent(float spent)
{
	timer_bar_spent = std::clamp(spent, 0.f, 1.f);
}

void Toast::set_size(Point size)
{
	Widget::set_size(size);
	size.y -= TIMER_BAR_THICKNESS;
	message.set_all_sizes(size);
}

void Toast::draw()
{
	draw_rounded_rect(Point(), get_size(), ELEMENT_ROUNDNESS, TOAST_COLOR);
	draw_widget(message);

	draw_rect(
		Point(0, message.get_size().y),
		Point(get_size().x * (1 - timer_bar_spent), TIMER_BAR_THICKNESS),
		TIMER_BAR_COLOR
	);
}
