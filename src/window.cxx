#include "raylib/raylib.h"

#include "theme.hxx"
#include "point.hxx"
#include "event.hxx"
#include "widget.hxx"
#include "container.hxx"
#include "window.hxx"
#include "utils.hxx"
#include "space_mono.bin.h"

using namespace eggui;

Font EG_MONO_FONT;

void Window::main_loop(int width_hint, int height_hint)
{
	// We draw frames only when something changes.
	// A change is defined as:
	//     A widget acknowledges responding to an event we sent to it.
	//     State of the window changes.

	root_container->set_position(Point(0, 0));
	root_container->layout_children();
	auto min_size = root_container->get_size();

	// Only set requested size if constraints satisfy
	// TODO verify more constraints which are required like
	//      max size, min size and resizability.
	auto size = min_size;
	if (width_hint >= size.x && height_hint >= size.y)
		size = Point(width_hint, height_hint);

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(size.x, size.y, title);

	// if (min_size.x <= width_hint && min_size.y <= height_hint) {
	// 	SetWindowMinSize(min_size.x, min_size.y);
	// } else {
	// 	TraceLog(
	// 		LOG_WARNING,
	// 		"%s: Provided window size(%dx%d) is too small.\n"
	// 		"Minimum size should be %dx%d for this configuration.",
	// 		__func__, width_hint, height_hint, min_size.x, min_size.y
	// 	);
	// 	SetWindowMinSize(width_hint, height_hint);
	// }
	EnableEventWaiting(); // Set sleep till a new event arrives
	SetExitKey(KEY_NULL); // Do not exit on ESC
	SetWindowMinSize(min_size.x, min_size.y);
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

void Window::update()
{
	if (IsWindowResized()) {
		needs_redraw = true;
		root_container->set_size(GetScreenWidth(), GetScreenHeight());
		return;
	}

	Point mpos = vec2_to_point(GetMousePosition());

	// Sends event and records the widget's response and returns the widget.
	auto notify = [this, mpos](Widget *w, EventType type, Point pd = Point()) {
		auto ev = Event(type);
		ev.cursor = mpos;
		ev.shared_pt_ = pd;
		auto resp = w->notify(ev);
		needs_redraw = resp ? true : needs_redraw;
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
		if (mouse_down_over == hovering_over)
			notify(mouse_down_over, EventType::MouseClick);

		notify(mouse_down_over, EventType::MouseReleased);
		mouse_down_over = nullptr;
		return;
	}

	Widget *hovered = nullptr;
	if (root_container->collides_with_point(mpos))
		hovered = notify(root_container.get(), EventType::IsInteractive);

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

	// Mouse scroll event, both axes.
	auto scroll = vec2_to_point(GetMouseWheelMoveV());
	if (scroll.x != 0 || scroll.y != 0)
		notify(hovered, EventType::Scroll, scroll);
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