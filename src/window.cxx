#include "raylib/raylib.h"

#include "window.hxx"
#include "widget.hxx"
#include "theme.hxx"
#include "container.hxx"
#include "graphics.hxx"

using namespace eggui;

constexpr int DEFAULT_UI_FPS = 30;

inline Point vec2_to_point(Vector2 v) { return Point(v.x, v.y); };

void Window::main_loop(int width_hint, int height_hint)
{
	// We draw frames only when something changes.
	// A change is defined as:
	//     A widget acknowledges responding to an event we sent to it.
	//     State of the window changes.

	layout(Point(width_hint, height_hint));
	auto size = root_container->get_size();

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(size.x, size.y, title);

	if (!IsWindowState(FLAG_VSYNC_HINT))
		SetTargetFPS(DEFAULT_UI_FPS);

	set_resize_limits();

	SetExitKey(KEY_NULL); // Do not exit on ESC.
	EnableEventWaiting(); // Sleep till a new event arrives.

	init_graphics();

	while (!(WindowShouldClose() && close_action())) {
		// Poll for events manually if nothing is drawn, since
		// when something is drawn events are polled automatically.
		if (draw_cnt-- > 0)
			draw();
		else
			PollInputEvents();

		update();
	}

	deinit_graphics();
	CloseWindow();
}

void Window::update()
{
	if (IsWindowResized()) {
		layout(Point(GetScreenWidth(), GetScreenHeight()));
		set_resize_limits();
		return;
	}

#ifndef NDEBUG
	if (IsKeyPressed(KEY_ESCAPE)) {
		debug_borders_enabled = !debug_borders_enabled;
		draw_cnt = 1;
		return;
	}
#endif

	Point mpos = vec2_to_point(GetMousePosition());

	// Sends event and records the widget's response and returns the widget.
	auto notify = [this, mpos](Widget *w, EventType type, Point pd = Point()) {
		auto ev = Event(type, mpos);
		ev.delta = pd;
		auto widget = w->notify(ev);
		draw_cnt = widget ? 1 : draw_cnt;
		return widget;
	};

	// We always send the scroll event, since it is used by scrollable views,
	// which are not interactive.
	// Mouse scroll event, both axes.
	auto scroll = vec2_to_point(GetMouseWheelMoveV());
	if (scroll.x != 0 || scroll.y != 0)
		notify(root_container.get(), EventType::Scroll, scroll);

	Widget *hovered = nullptr;
	if (root_container->collides_with_point(mpos))
		hovered = notify(root_container.get(), EventType::IsInteractive);

	// Detect if mouse released, clicked or dragged. It can only happen if
	// mouse button was already pressed over some widget and that widget
	// responded to the button press.
	if (mouse_down_over) {
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			// Register a click only if mouse button is released while the
			// cursor is over the same widget, otherwise ignore the click.
			if (mouse_down_over == hovered)
				notify(mouse_down_over, EventType::MouseClick);

			notify(mouse_down_over, EventType::MouseReleased);
			mouse_down_over = nullptr;
		} else {
			// If mouse button has not been released then, we act as if the
			// cursor has not left the widget even if the cursor is really
			// hovering over some another widget.
			hovered = mouse_down_over;

			auto delta = vec2_to_point(GetMouseDelta());
			if (delta.x != 0 || delta.y != 0)
				notify(mouse_down_over, EventType::MouseDrag, delta);
		}
	}

	// If a new widget(or none) is being hovered over then notify the
	// older widget that it is no longer being hovered over.
	if (hovering_over && hovering_over != hovered) {
		notify(hovering_over, EventType::MouseOut);
		hovering_over = nullptr;
	}

	if (!hovered)
		return;

	// If we are hoveing over the same widget then notify it about where
	// the mouse is hovering over it along with its delta.
	// Else notify the newly hovered over widget that it is being hovered.
	if (hovering_over == hovered) {
		auto delta = vec2_to_point(GetMouseDelta());
		notify(hovered, EventType::MouseMotion, delta);
	} else {
		hovering_over = notify(hovered, EventType::MouseIn);
	}

	// Notify that the widget has been pressed by left mouse.
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		mouse_down_over = notify(hovered, EventType::MousePressed);
}

void Window::draw()
{
	BeginDrawing();

	clear_background();

	root_container->draw();
	if (debug_borders_enabled)
		root_container->draw_debug();

	EndDrawing();
}

void Window::layout(Point size)
{
	// HACK - We draw twice when resized.
	// Drawing only once causes small black square shaped boxes to appear
	// at top-right and bottom-left corners and the drawing of that part to
	// be shifted.
	draw_cnt = 2;
	root_container->set_size(size);
	root_container->set_position(Point(0, 0));
}

void Window::set_resize_limits()
{
	if (!IsWindowReady())
		return;

	const Point win_min(1, 1);
	const Point win_max(
		GetMonitorWidth(GetCurrentMonitor()),
		GetMonitorHeight(GetCurrentMonitor())
	);

	auto minsz = root_container->get_min_size();
	auto maxsz = root_container->get_max_size();
	minsz = clamp_components(minsz, win_min, win_max);
	maxsz = clamp_components(maxsz, win_min, win_max);

	SetWindowMinSize(minsz.x, minsz.y);
	SetWindowMaxSize(maxsz.x, maxsz.y);
}