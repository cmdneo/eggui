#include <cassert>
#include <cstdio>

#include "widget.hxx"
#include "theme.hxx"
#include "container.hxx"
#include "graphics.hxx"
#include "canvas.hxx"

namespace eggui
{
void draw_widget(Widget &w)
{
	const auto pen = w.canvas.acquire_pen();
	w.is_drawing_visible = w.is_visible(pen);
	if (w.is_drawing_visible)
		w.draw();
}

void draw_widget_debug(Widget &w)
{
	// Always draw debug info without any clipping
	const auto pen = w.canvas.acquire_pen(false);
	w.draw_debug();
}

Widget *notify_widget(Widget &w, Event ev)
{
	// Make cursor position relative to the widget and constrain it within
	// the widget boundary.
	ev.cursor -= w.get_position();
	ev.cursor = clamp_components(ev.cursor, Point(0, 0), w.get_size());
	return w.notify(ev);
}

void Widget::set_size(Point new_size)
{
	assert(get_min_size().x <= new_size.x && get_min_size().y <= new_size.y);
	assert(get_max_size().x >= new_size.x && get_max_size().y >= new_size.y);

	canvas.set_size(new_size);
}

void Widget::set_all_sizes(Point size)
{
	set_max_size(size);
	set_min_size(size);
	set_size(size);
}

void Widget::set_position(Point new_pos) { canvas.set_position(new_pos); }

Point Widget::calc_abs_position() const
{
	Point ret = get_position();

	for (auto p = get_parent(); p; p = p->get_parent())
		ret += p->get_position();

	return ret;
}

void Widget::draw_debug()
{
	draw_rect_lines(Point(), get_size(), DEBUG_BORDER_COLOR);

#ifdef EGGUI_DRAW_DEBUG_POSITIONS
	static char buffer[256];
	auto pos = calc_abs_position();
	std::snprintf(buffer, sizeof buffer, "(%d, %d)", pos.x, pos.y);

	// Paint the position red if widget is not visible or gets clipped.
	RGBA color = RGBA(255, 128, 0);
	if (!is_drawing_visible)
		color.g = 0;

	draw_text(Point(0, 0), color, buffer, FontSize::Tiny);
#endif
}

bool Interactive::handle_mouse_hover_events(Event ev)
{
	if (is_disabled())
		return false;

	switch (ev.type) {
	case EventType::MouseIn:
		is_hovering = true;
		break;
	case EventType::MouseOut:
		is_hovering = false;
		break;

	default:
		return false;
		break;
	}

	return true;
}

bool Interactive::handle_mouse_press_events(Event ev)
{
	if (is_disabled())
		return false;

	switch (ev.type) {
	case EventType::MousePressed:
		is_pressed = true;
		break;
	case EventType::MouseReleased:
		is_pressed = false;
		break;

	default:
		return false;
	}

	return true;
}

Widget *Interactive::notify(Event ev)
{
	if (!is_disabled() && ev.type == EventType::IsInteractive)
		return this;
	return nullptr;
}

bool Widget::is_visible(const Pen &pen) const
{
	assert(canvas.has_active_pen());

	// All widgets are drawn with clipping.
	auto size = pen.get_clip_region().second;
	return size.x * size.y >= 1;
}
} // namespace eggui