#include "raylib/raylib.h"

#include "theme.hxx"
#include "point.hxx"
#include "event.hxx"
#include "widget.hxx"
#include "container.hxx"
#include "window.hxx"

using namespace eggui;

static constexpr int INPUT_POLL_INTERVAL_MS = 20;

void Window::main_loop(int width, int height)
{
	// We draw frames only when something changes.
	// A change is defined as:
	//     A widget acknowledges responding to an event we sent to it.
	//     State of the window changes.

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
	InitWindow(width, height, title);
	SetExitKey(KEY_NULL);

	root_container->set_size(width, height);

	while (1) {
		update();

		// Only draw if something changes, otherwise wait and manually poll for events.
		if (needs_redraw) {
			draw();
		} else {
			WaitTime(INPUT_POLL_INTERVAL_MS / 1000.);
			PollInputEvents();
		}

		// This needs to be placed after draw to work, because inputs are
		// polled after EndDrawing, or we manually poll them.
		if (WindowShouldClose() && close_action())
			break;
	}

	CloseWindow();
}

static bool is_point_in_box(Point pt, Point box_pos, Point box_size)
{
	Point start = box_pos;
	Point end = box_pos + box_size;

	return pt.x >= start.x && pt.x < end.x && pt.y >= start.y && pt.y < end.y;
}

static Point vec2_to_point(Vector2 v) { return Point(v.x, v.y); }

void Window::update()
{
	// TODO make resize propogate
	if (IsWindowResized()) {
		needs_redraw = true;
		root_container->set_size(GetScreenWidth(), GetScreenHeight());
	}

	Point mpos = vec2_to_point(GetMousePosition());
	Widget *hovered = root_container.get();

	// Find the innermost widget over which the mouse is, without it if
	// we hover over anything contained in a container, we will always get
	// the container instead of the actual widget we are hovering over.
	while (1) {
		Widget *newer = nullptr;
		for (auto &c : hovered->children) {
			if (is_point_in_box(mpos, c->get_position(), c->get_size())) {
				newer = c.get();
				break;
			}
		}

		if (!newer)
			break;
		hovered = newer;
	}

	// Notifes the object of the event.
	auto notify = [this, mpos](Widget *w, EventType type, Point pd = Point()) {
		auto ev = Event(type);
		ev.cursor = mpos;
		ev.shared_pt_ = pd;
		needs_redraw = w->notify(ev) || needs_redraw;
	};

	if (mouse_down_over) {
		if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			auto delta = vec2_to_point(GetMouseDelta());
			if (delta.x != 0 || delta.y != 0)
				notify(mouse_down_over, EventType::MouseDrag, delta);

			return;
		}

		// Register a click only if mouse button is released while the cursor is
		// over the same widget, otherwise ignore the click.
		if (mouse_down_over == hovered)
			notify(mouse_down_over, EventType::MouseClick);

		notify(mouse_down_over, EventType::MouseReleased);
		mouse_down_over = nullptr;
		return;
	}

	// If a new widget(or none) is being hovered over then notify the
	// older widget that it is no longer being hovered over.
	if (hovering_over && hovering_over != hovered)
		notify(hovering_over, EventType::MouseOut);

	if (!hovered) {
		hovering_over = nullptr;
		return;
	}

	// Notify that the widget has been pressed by left mouse.
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		mouse_down_over = hovered;
		notify(hovered, EventType::MousePressed);
	}
	// Notify that the widget is being hovered over only if we were not
	// previously hovering over the same widget.
	if (hovering_over != hovered)
		notify(hovered, EventType::MouseIn);

	// TODO Use Scroll event for the Scrollable widget

	hovering_over = hovered;
	return;
}

void Window::draw()
{
	needs_redraw = false;

	BeginDrawing();
	ClearBackground(BACKGROUND_COLOR);

	root_container->draw();
	if (debug_borders_enabled)
		root_container->draw_debug();

	EndDrawing();
}