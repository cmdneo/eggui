#include <climits>

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
	auto size = root_widget->get_size();

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(size.x, size.y, title);

	if (!IsWindowState(FLAG_VSYNC_HINT))
		SetTargetFPS(DEFAULT_UI_FPS);

	set_resize_limits();

	SetExitKey(KEY_NULL); // Do not exit on ESC.
	EnableEventWaiting(); // Sleep untill a new input event arrives.
	event_waiting_enabled = true;

	init_graphics();

	while (!(WindowShouldClose() && close_action())) {
		// Poll for events manually if nothing is drawn, since
		// when something is drawn events are polled automatically.
		if (!event_waiting_enabled || draw_cnt-- > 0)
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

	// TODO Put this animation update in a better place
	// If there are any animations pending then disable event waiting.
	// if (!animations.empty() && event_waiting_enabled) {
	// 	DisableEventWaiting();
	// 	event_waiting_enabled = false;
	// } else if (!event_waiting_enabled) {
	// 	EnableEventWaiting();
	// 	event_waiting_enabled = true;
	// }

	// Run all the animations.
	// for (auto &a : animations)
	// 	a.update();
	// swap_remove_if(animations, [](const Animation &a) {
	// 	return a.has_ended();
	// });

	handle_mouse_events();

	if (!hovering_over)
		return;

	// TODO Cleanup this keyboard testing stuff
	int charc = GetCharPressed();
	if (charc != 0) {
		Event ev(*this, EventType::CharEntered, charc);
		send_event(hovering_over, ev);
	}
	static int keyc = 0;
	keyc = GetKeyPressed();
	if (charc == 0 && keyc != KEY_NULL) {
		Event ev(*this, EventType::KeyPressed, keyc);
		send_event(hovering_over, ev);
	};
}

void Window::draw()
{
	BeginDrawing();

	clear_background();

	root_widget->draw();
	if (debug_borders_enabled)
		root_widget->draw_debug();

	EndDrawing();
}

void Window::layout(Point size)
{
	// HACK - We draw twice when resized.
	// Drawing only once causes small black square shaped boxes to appear
	// at top-right and bottom-left corners and the drawing of that part to
	// be shifted.
	draw_cnt = 2;
	root_widget->set_size(size);
	root_widget->set_position(Point(0, 0));
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

	auto minsz = root_widget->get_min_size();
	auto maxsz = root_widget->get_max_size();
	minsz = clamp_components(minsz, win_min, win_max);
	maxsz = clamp_components(maxsz, win_min, win_max);

	SetWindowMinSize(minsz.x, minsz.y);
	SetWindowMaxSize(maxsz.x, maxsz.y);
}

void Window::handle_mouse_events()
{
	// We always send the scroll event, since it is used by
	// scrollable views, which are not interactive in a general way.
	// TODO Better classify which widgets can recieve which events.
	auto scroll = vec2_to_point(GetMouseWheelMoveV());
	if (scroll.x != 0 || scroll.y != 0)
		notify(root_widget.get(), EventType::Scroll, scroll);

	Widget *hovered = nullptr;
	// Check if the widget is interactive by sending a special event,
	// then only it can be interacted with using the mouse/keyboard.
	auto mpos = vec2_to_point(GetMousePosition());
	if (root_widget->collides_with_point(mpos))
		hovered = notify(root_widget.get(), EventType::IsInteractive);

	// If mouse is not down over anything and then the
	// mouse button is pressed and the widget responds to it.
	if (!mouse_down_over) {
		if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			mouse_down_over = notify(hovered, EventType::MousePressed);
	}
	// If mouse released.
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		// Register a click only if the mouse button is released while
		// hovering over the same widget it was pressed upon.
		if (mouse_down_over == hovered)
			notify(mouse_down_over, EventType::MouseClick);

		notify(mouse_down_over, EventType::MouseReleased);
		mouse_down_over = nullptr;
	}
	// If the mouse button was pressed over some widget and has not been
	// released yet, then, we act as if the cursor has not left the widget
	// even if the cursor is might be hovering over some another widget.
	else {
		hovered = mouse_down_over;

		auto delta = vec2_to_point(GetMouseDelta());
		if (delta.x != 0 || delta.y != 0)
			notify(mouse_down_over, EventType::MouseDrag, delta);
	}

	// If a new widget(or none) is being hovered over then notify the
	// older widget that it is no longer being hovered over.
	if (hovering_over && hovering_over != hovered) {
		notify(hovering_over, EventType::MouseOut);
		hovering_over = nullptr;
	}

	if (!hovered)
		return;

	// If we are still hovering over the same widget then notify it about
	// where the mouse is hovering over it along with its delta. Otherwise,
	// notify the new widget that it is being hovered over.
	if (hovering_over == hovered) {
		auto delta = vec2_to_point(GetMouseDelta());
		notify(hovered, EventType::MouseMotion, delta);
	} else {
		notify(hovered, EventType::MouseIn);
		hovering_over = hovered;
	}
}

Widget *Window::notify(Widget *w, EventType type, Point extra)
{
	auto mpos = vec2_to_point(GetMousePosition());
	auto ev = Event(*this, type, mpos);
	ev.delta = extra;
	return send_event(w, ev);
}

Widget *Window::send_event(Widget *w, Event ev)
{
	auto ret = w->notify(ev);
	draw_cnt = ret ? 1 : draw_cnt;
	return ret;
}