#include <cassert>

#include "raylib/raylib.h"

#include "graphics.hxx"
#include "window.hxx"
#include "widget.hxx"
#include "theme.hxx"
#include "utils/swap_remove.hxx"

using namespace eggui;

inline Point vec2_to_point(Vector2 v) { return Point(v.x, v.y); };

inline Point get_mouse_pos() { return vec2_to_point(GetMousePosition()); }

void Window::set_title(std::string title_str)
{
	title = std::move(title_str);
	if (is_running)
		SetWindowTitle(title.c_str());
}

void Window::main_loop(int width_hint, int height_hint)
{
	// We draw frames only when something changes.
	// A change is defined as:
	//     A widget acknowledges responding to an event we sent to it.
	//     State of the window changes.
	//     A new animation frame is required.

	layout(Point(width_hint, height_hint));
	auto size = root_widget->get_size();

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	InitWindow(size.x, size.y, title.c_str());
	init_graphics();

	// This is not really needed since we also check for frame timing manually,
	// but just in case the default FPS is too low and it sleeps for too long.
	if (!IsWindowState(FLAG_VSYNC_HINT))
		SetTargetFPS(TICKS_PER_SECOND);

	set_resize_limits();

	SetExitKey(KEY_NULL); // Do not exit on ESC.
	EnableEventWaiting(); // Sleep untill a new input event arrives.
	event_waiting_enabled = true;
	last_update_time = GetTime();
	is_running = true;

	while (!((WindowShouldClose() || close_requested) && close_action(*this))) {
		update();
		last_update_time = GetTime();

		// Poll for events manually when nothing is drawn, since when we draw
		// events are polled by the draw method.
		if (draw_cnt > 0) {
			draw_cnt--;
			draw();
		} else {
			PollInputEvents();
		}

		// Manage frame timing as per the update interval, sleep if time left.
		if (auto extra = UPDATE_DELTA_TIME - get_update_dt(); extra > 0) {
			WaitTime(extra);
		}
	}

	is_running = false;
	deinit_graphics();
	CloseWindow();
}

void Window::add_animation(Widget *w, Animation animation)
{
	animations.push_back({w, animation});
}

void Window::remove_animations(Widget *w)
{
	swap_remove_if(animations, [w](const auto &anim) {
		return anim.first == w;
	});
}

void Window::add_overlay(std::shared_ptr<Widget> w)
{
	overlays.push_back({std::move(w), false});
}

void Window::remove_overlay(Widget *w)
{
	for (auto &ov : overlays) {
		if (ov.widget.get() == w) {
			ov.removed = true;
			break;
		}
	}

	assert(!"Overlay to be removed does not exist.");
}

void Window::request_focus(Interactive *w, bool keep_pinned)
{
	keep_focus_pinned = keep_pinned;

	// Do not change anything if already focused.
	if (w == focused_on)
		return;

	if (focused_on)
		notify_n_ack(focused_on, EventType::FocusLost);

	focused_on = w;
	notify_n_ack(focused_on, EventType::FocusGained);
}

void Window::request_close_window(Widget *w)
{
	assert(w);
	close_requested = !!w;
}

void Window::update()
{
	// If window is resized then just re-layout and ignore any other events.
	if (IsWindowResized()) {
		// HACK - We draw twice when resized.
		// Drawing only once causes small black square shaped boxes to appear
		// at top-right and bottom-left corners and the drawing of that part to
		// be shifted. This only happend when the window is maximized.
		draw_cnt = 2;

		layout(Point(GetScreenWidth(), GetScreenHeight()));
		set_resize_limits();
		return;
	}

#ifndef NDEBUG
	if (IsKeyPressed(KEY_ESCAPE)) {
		debug_borders_enabled = !debug_borders_enabled;
		draw_cnt = 1;
	}
#endif
	// Any new animations added by event handlers will be started in the
	// next update call.
	play_animations();
	handle_mouse_events();
	handle_keyboard_events();

	// If there are any animations pending then keep event waiting disabled.
	// Also start the animation timer to ensure accurate animation step
	// timings regardless of the update interval.
	if (!animations.empty()) {
		if (event_waiting_enabled) {
			DisableEventWaiting();
			animation_lag = 0;
			event_waiting_enabled = false;
		}
	} else if (!event_waiting_enabled) {
		EnableEventWaiting();
		event_waiting_enabled = true;
	}

	// Remove the overlays which have been marked for removal.
	swap_remove_if(overlays, [](const auto &overlay) {
		return overlay.removed;
	});
}

void Window::draw()
{
	BeginDrawing();

	clear_background();

	draw_widget(*root_widget);
	if (debug_borders_enabled)
		draw_widget_debug(*root_widget);

	auto parent_abs_pos = [](Widget &w) {
		if (auto p = w.get_parent())
			return p->calc_abs_position();
		else
			return Point(0, 0);
	};

	// Overlays are drawn where they would appear normally inside their of
	// parent widget but are placed on top of every other non-overlay widget.
	// Overlays thus may shadow other non-overlay widgets.
	for (auto &ov : overlays) {
		push_translation(parent_abs_pos(*ov.widget));

		draw_widget(*ov.widget);
		if (debug_borders_enabled)
			draw_widget_debug(*ov.widget);

		pop_translation();
	}

	EndDrawing();
}

void Window::layout(Point size)
{
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
	// TODO Decide if we should send scroll to an overlay.
	// We always send the scroll event, since it is used by scrollable views,
	// which are just containers and are not interactive in a general way.
	auto scroll = vec2_to_point(GetMouseWheelMoveV());
	if (scroll.x != 0 || scroll.y != 0)
		notify_n_ack(root_widget.get(), EventType::Scroll, scroll);

	Widget *hovered = nullptr;

	auto check_hovering = [&](Widget &w) {
		if (!w.collides_with_point(get_mouse_pos()))
			return;
		auto ev = Event(*this, EventType::IsInteractive, get_mouse_pos());
		hovered = notify_widget(w, ev);
	};

	// First check if we are hovering over any of the
	// floating-widgets(overlays) and then for the root_widget.
	for (auto &ov : overlays) {
		check_hovering(*ov.widget);
		if (hovered)
			break;
	}
	if (!hovered) {
		check_hovering(*root_widget);
	}

	// *** Handle mouse button press/release and drag ***
	if (!mouse_down_over) {
		// Some widget responds to the mouse press.
		if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			mouse_down_over = notify_n_ack(hovered, EventType::MousePressed);
	}
	// If mouse released while it was down over some widget.
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		// Register a click only if the mouse button is released while
		// hovering over the same widget it was pressed upon.
		if (mouse_down_over == hovered)
			notify_n_ack(mouse_down_over, EventType::MouseClick);

		notify_n_ack(mouse_down_over, EventType::MouseReleased);
		mouse_down_over = nullptr;
	}
	// If the mouse button was pressed over some widget and has not been
	// released yet, then, we act as if the cursor has not left the widget
	// even if the cursor might be hovering over some another widget.
	else {
		hovered = mouse_down_over;
		// Mouse moved while a mouse button is pressed over the widget.
		auto delta = vec2_to_point(GetMouseDelta());
		if (delta.x != 0 || delta.y != 0)
			notify_n_ack(mouse_down_over, EventType::MouseDrag, delta);
	}

	// If some widget had acquired focus earlier but then the mouse button is
	// pressed over some another widget and it has not been requested to keep
	// the focus pinned, then it loses its focus.
	// Since hovered can be a nullptr, we check for button press explicitly.
	if (!keep_focus_pinned && focused_on && hovered != focused_on
		&& IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		notify_n_ack(focused_on, EventType::FocusLost);
		focused_on = nullptr;
	}

	// If a new widget(or none) is being hovered over then notify the
	// older widget that it is no longer being hovered over.
	if (hovering_over && hovering_over != hovered) {
		notify_n_ack(hovering_over, EventType::MouseOut);
		hovering_over = nullptr;
	}

	if (!hovered)
		return;

	// If we are still hovering over the same widget then notify it about
	// where the mouse is hovering over it along with its delta. Otherwise,
	// notify the new widget that it is being hovered over.
	if (hovering_over == hovered) {
		auto delta = vec2_to_point(GetMouseDelta());
		notify_n_ack(hovered, EventType::MouseMotion, delta);
	} else {
		notify_n_ack(hovered, EventType::MouseIn);
		hovering_over = hovered;
	}
}

void Window::handle_keyboard_events()
{
	if (!focused_on)
		return;

	// TODO Cleanup this keyboard testing stuff
	int charc = GetCharPressed();
	if (charc != 0) {
		Event ev(*this, EventType::CharEntered, charc);
		notify_widget(*focused_on, ev);
	}

	int keyc = GetKeyPressed();
	if (charc == 0 && keyc != KEY_NULL) {
		Event ev(*this, EventType::KeyPressed, keyc);
		notify_widget(*focused_on, ev);
	};
}

void Window::play_animations()
{
	// Fixed step timing method.
	// Advance frames according to time accumulated.
	animation_lag += get_update_dt();

	while (animation_lag >= UPDATE_DELTA_TIME) {
		for (auto &a : animations) {
			if (!a.second.has_ended())
				draw_cnt = a.second.update() ? 1 : draw_cnt;
		}

		animation_lag -= UPDATE_DELTA_TIME;
	}

	swap_remove_if(animations, [](const auto &a) {
		return a.second.has_ended();
	});
}

Widget *Window::notify_n_ack(Widget *w, EventType type, Point extra)
{
	assert(w);

	auto ev = Event(*this, type, get_mouse_pos());
	ev.delta = extra;

	auto ret = notify_widget(*w, ev);
	draw_cnt = ret && draw_cnt == 0 ? 1 : draw_cnt;

	return ret;
}

double Window::get_update_dt() const { return GetTime() - last_update_time; }
