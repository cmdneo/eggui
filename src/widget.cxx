#include <cassert>
#include <cstdio>

#include "widget.hxx"
#include "theme.hxx"
#include "container.hxx"
#include "graphics.hxx"
#include "canvas.hxx"

using namespace eggui;

void Widget::set_size(Point new_size)
{
	assert(get_min_size().x <= new_size.x && get_min_size().y <= new_size.y);
	assert(get_max_size().x >= new_size.x && get_max_size().y >= new_size.y);

	canvas.set_size(new_size);
}

void Widget::set_position(Point new_pos) { canvas.set_position(new_pos); }

Point Widget::calc_abs_position() const
{
	Point ret = get_position();

	Widget *p = parent;
	while (p) {
		ret += p->get_position();
		p = p->parent;
	}

	return ret;
}

Widget *Widget::notify(Event ev)
{
	// Make cursor position relative to the widget.
	ev.cursor -= get_position();
	return notify_impl(ev);
}

void Widget::draw_debug()
{
	// Always draw debug info without any clipping
	const auto pen = canvas.acquire_pen(false);
	draw_debug_impl();
}

void Widget::draw()
{
	const auto pen = canvas.acquire_pen();
	is_drawing_visible = is_visible(pen);
	if (is_drawing_visible)
		draw_impl();
};

void Widget::draw_debug_impl()
{
	draw_rect_lines(Point(), get_size(), DEBUG_BORDER_COLOR);

	static char buffer[256];
	auto pos = calc_abs_position();
	snprintf(buffer, sizeof buffer, "(%.1f, %.1f)", pos.x, pos.y);

	RGBA color = RGBA(255, 128, 0);
	if (!is_drawing_visible)
		color.g = 0;

	draw_text(Point(0, 0), color, buffer, FontSize::Tiny);
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
	case EventType::MouseMotion:
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

Widget *Interactive::notify_impl(Event ev)
{
	if (!is_disabled() && ev.type == EventType::IsInteractive)
		return this;
	return nullptr;
}

bool Widget::is_visible(const Pen &pen) const
{
	assert(canvas.has_active_pen());

	// All widgets are drawn with clipping, so if a widget lies outside of
	// the clip area then it is not visible.
	auto [pos, size] = pen.get_clip_region();
	return check_box_collision(pos, size, Point(0, 0), get_size());
}
