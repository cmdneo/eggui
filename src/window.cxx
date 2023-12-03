#include "raylib/raylib.h"

#include "theme.hxx"
#include "point.hxx"
#include "event.hxx"
#include "widget.hxx"
#include "container.hxx"
#include "window.hxx"

using namespace eggui;

void Window::main_loop(int width, int height)
{
	// We draw frames only when something changes.
	// A change is defined as:
	//     A widget acknowledges responding to an event we sent to it.
	//     State of the window changes.

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
	InitWindow(width, height, title);

	auto min_size = root_container->calc_min_size();
	root_container->set_size(width, height);

	EnableEventWaiting(); // Set sleep till a new event arrives
	SetExitKey(KEY_NULL); // Do not exit on ESC
	SetWindowMinSize(min_size.x, min_size.y);

	while (1) {
		update();

		if (needs_redraw)
			draw();
		else
			PollInputEvents();

		// This needs to be placed after we poll for events to work properly.
		if (WindowShouldClose() && close_action())
			break;
	}

	CloseWindow();
}

static Point vec2_to_point(Vector2 v) { return Point(v.x, v.y); }

void Window::update()
{
	if (IsWindowResized()) {
		needs_redraw = true;
		root_container->set_size(GetScreenWidth(), GetScreenHeight());
		return;
	}

	Point mpos = vec2_to_point(GetMousePosition());
	Widget *hovered = root_container->get_active_widget_at(mpos);

	// Sends event and records and returns the widget's response to the event.
	auto notify = [this, mpos](Widget *w, EventType type, Point pd = Point()) {
		auto ev = Event(type);
		ev.cursor = mpos;
		ev.shared_pt_ = pd;
		bool resp = w->notify(ev);
		needs_redraw = resp || needs_redraw;
		return resp;
	};

	// Detect if mouse released, clicked or dragged. It can only happen if
	// mouse button was already pressed over some widget and that widget
	// responded to the button press.
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
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
		&& notify(hovered, EventType::MousePressed))
		mouse_down_over = hovered;

	// Notify that the widget is being hovered over only if we were not
	// previously hovering over the same widget.
	if (hovering_over != hovered) {
		if (notify(hovered, EventType::MouseIn))
			hovering_over = hovered;
		else
			hovering_over = nullptr;
	}

	// TODO Use Scroll event for the Scrollable widget
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