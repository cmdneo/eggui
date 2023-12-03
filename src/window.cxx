#include "raylib/raylib.h"

#include "theme.hxx"
#include "point.hxx"
#include "event.hxx"
#include "widget.hxx"
#include "container.hxx"
#include "window.hxx"
#include "space_mono.bin.h"

using namespace eggui;

Font EG_MONO_FONT;

void Window::main_loop(int width, int height)
{
	// We draw frames only when something changes.
	// A change is defined as:
	//     A widget acknowledges responding to an event we sent to it.
	//     State of the window changes.

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(width, height, title);

	auto min_size = root_container->calc_min_size();
	root_container->set_size(width, height);

	if (min_size.x <= width && min_size.y <= height) {
		SetWindowMinSize(min_size.x, min_size.y);
	} else {
		TraceLog(
			LOG_WARNING,
			"%s: Provided window size(%dx%d) is too small.\n"
			"Minimum size should be %dx%d for this configuration.",
			__func__, width, height, min_size.x, min_size.y
		);
		SetWindowMinSize(width, height);
	}

	EnableEventWaiting(); // Set sleep till a new event arrives
	SetExitKey(KEY_NULL); // Do not exit on ESC
	EG_MONO_FONT = LoadFontFromMemory(
		".ttf", SPACE_MONO_REGULAR_TTF, SPACE_MONO_REGULAR_TTF_LEN,
		EG_FONT_SIZE, nullptr, 0
	);

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

	UnloadFont(EG_MONO_FONT);
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

	// Sends event and records the widget's response to the event.
	auto notify = [this, mpos](Widget *w, EventType type, Point pd = Point()) {
		auto ev = Event(type);
		ev.cursor = mpos;
		ev.shared_pt_ = pd;
		bool resp = w->notify(ev);
		needs_redraw = resp || needs_redraw;
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
	if (hovering_over && hovering_over != hovered) {
		notify(hovering_over, EventType::MouseOut);
		hovering_over = nullptr;
	}

	if (!hovered)
		return;

	// Notify that the widget is being hovered over only if we were not
	// previously hovering over the same widget.
	if (hovering_over != hovered) {
		notify(hovered, EventType::MouseIn);
		hovering_over = hovered;
	}

	// Notify that the widget has been pressed by left mouse.
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		notify(hovered, EventType::MousePressed);
		mouse_down_over = hovered;
	}

	// TODO Use Scroll event for the Scrollable widget
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