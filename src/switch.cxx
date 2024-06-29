#include <algorithm>

#include "window.hxx"
#include "animation.hxx"
#include "switch.hxx"
#include "theme.hxx"

using namespace eggui;

inline int map_range(int min, int max, float frac)
{
	return min + frac * (max - min);
}

void Switch::set_state(bool new_state, Window &window)
{
	if (state != new_state) {
		state = new_state;
		on_toggle(window, *this, state);
	}

	auto anim_cb = [this, init_pos = slider_pos](int, float p) {
		slider_pos = state ? init_pos + p : init_pos - p;
		slider_pos = std::clamp(slider_pos, 0.f, 1.f);
		return true;
	};
	window.remove_animations(this);
	window.add_animation(
		this, Animation(TICKS_PER_SECOND / 5, 0, false, anim_cb)
	);
}

Widget *Switch::notify_impl(Event ev)
{
	if (handle_mouse_hover_events(ev))
		return this;
	if (handle_mouse_press_events(ev))
		return this;

	if (ev.type == EventType::MouseClick) {
		set_state(!state, ev.window);
		return this;
	}

	return Interactive::notify_impl(ev);
}

void Switch::draw_impl()
{

	auto rad = get_size().y / 2 - SWITCH_PADDING;
	auto xoffset = rad + SWITCH_PADDING;
	Point pos(
		map_range(xoffset, get_size().x - xoffset, slider_pos), get_size().y / 2
	);

	// Draw the sliding circle along with the ON switch color attached
	// before it for a sliding effect.
	draw_rounded_rect(Point(), get_size(), 1, SWITCH_OFF_BG);
	if (slider_pos > 0)
		draw_rounded_rect(
			Point(), Point(pos.x + xoffset, get_size().y), 1, SWITCH_ON_BG
		);

	draw_cirlce(pos, rad, SWITCH_SLIDER_COLOR);

	if (is_hovering)
		draw_rounded_rect(Point(), get_size(), 1, SWITCH_HOVER_TINT);
}
