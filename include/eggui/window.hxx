#ifndef WINDOW_HXX_INCLUDED
#define WINDOW_HXX_INCLUDED

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "widget.hxx"
#include "animation.hxx"
#include "toast.hxx"

namespace eggui
{
struct Overlay {
	std::shared_ptr<Widget> widget;
	// For deciding top position if an overlay overlap with other overlays.
	int z_index;
	// Indicate if the overlay has been marked for removal,
	// and will be removed at the end of the current update.
	bool removed;
};

class Window
{
public:
	Window(std::shared_ptr<Widget> container)
		: root_widget(std::move(container))
	{
	}

	/// @brief Set window title
	/// @param title_str Title string, null terminated.
	void set_title(std::string title_str);

	/// @brief Action to be performed when closing
	/// @param callback Callback, window is closed only if it returns true.
	void set_close_action(std::function<bool(Window &)> callback)
	{
		close_action = callback;
	}

	void set_debug(bool enable) { debug_borders_enabled = enable; }

	/// @brief Create the window and start handling input events.
	/// @param width_hint Desired window width (negative for auto).
	/// @param height_hint Desired window height (negative for auto).
	void main_loop(int width_hint = -1, int height_hint = -1);

	// Service request methods for widgets.
	//---------------------------------------------------------------

	/// @brief Attach an animation to the widget.
	/// @param w The widget which is requesting the animation.
	/// @param animation Animation frame callback object.
	void add_animation(Widget *w, Animation animation);
	/// @brief Remove all animations associated with the widget.
	/// @param w The widget.
	void remove_animations(Widget *w);

	/// @brief Add a floating widgte.
	/// @param w The overlay.
	/// @param z_index z-index in case it overlaps with other overlays.
	void add_overlay(std::shared_ptr<Widget> w, int z_index = 1);
	/// @brief Remove an floating widget.
	/// @param w The widget pointet for identification.
	void remove_overlay(Widget *w);

	/// @brief Provide focus to the widget, removing earlier focus if any.
	/// @param w The widget to be focused on, nullptr to remove current focus.
	/// @param keep_pinned Kepp focused until explicitly changed/removed by it.
	void request_focus(Interactive *w, bool keep_pinned);

	/// @brief Request the window to close the window and end its main loop.
	/// @param w Requesting widget.
	void request_close_window(Widget *w);

private:
	/// @brief Update all the widgets by sending events to them.
	void update();
	/// @brief Draw the window.
	void draw();

	/// @brief Layout the widgets.
	/// @param size Size of the window for layout.
	void layout(Point size);
	/// @brief Set min and max window size as per root_widget size.
	void set_resize_limits();

	/// @brief Handle mouse related events.
	void handle_mouse_events();
	/// @brief Handle key press events.
	void handle_keyboard_events();
	/// @brief Play all the animations and manage them
	void play_animations();

	/// @brief Send event to the widget along with current cursor position
	/// and record(as `draw_cnt > 0`) if the widget responded.
	/// @return The widget which responded to the event.
	Widget *notify_n_ack(Widget *w, EventType type, Point extra = Point(0, 0));
	/// @brief Send scroll(if any) to the widget.
	void send_scroll_to(Widget *w);
	/// @brief Get time elapsed since last update.
	/// @return Delta time.
	double get_update_dt() const;

	// Window title.
	std::string title = "EGGUI Window";
	// The top level widget, generally a container.
	std::shared_ptr<Widget> root_widget;
	// Function to be run if the user tries to close the window.
	// Returns true if the window should be closed immediately.
	std::function<bool(Window &)> close_action = [](Window &) { return true; };

	// Is the window active and main loop running.
	bool is_running = false;
	// UI debug mode setting.
	bool debug_borders_enabled = false;
	// Is sleep to wait for input events enabled.
	bool event_waiting_enabled = false;
	// Number of times widgets should be drawn after a change.
	int draw_cnt = 1;
	// Monotonic time when update was last called.
	double last_update_time = 0;

	// Widget over which mouse button has been pressed but not released yet.
	Widget *mouse_down_over = nullptr;
	// Widget over which the cursor is placed, if a mouse button has been
	// pressed over an interactive widget then, it will be same as `mouse_down_over`.
	Widget *hovering_over = nullptr;
	// Widget over which keyboard is focused, all keypressed will to sent to it.
	Widget *focused_on = nullptr;
	/// Do not remove focus until explicitly changed/removed by request_focus.
	bool keep_focus_pinned = false;
	// Has any widget has requested to close the window.
	bool close_requested = false;

	// Pending animations along with their associated widgets.
	std::vector<std::pair<Widget *, Animation>> animations;
	// Time by how much by animations are lagging behind from the present.
	// We use this to update animations frames using a fixed delta time.
	double animation_lag = 0;

	// Floating widgets, overlaid over the root widget.
	// Kept sorted in descending order on `z_index` field, overlays
	// with same `z_index` are then ordered on time added(most recent first).
	std::vector<Overlay> overlays;
};
} // namespace eggui

#endif
