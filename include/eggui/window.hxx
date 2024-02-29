#ifndef WINDOW_HXX_INCLUDED
#define WINDOW_HXX_INCLUDED

#include <memory>
#include <utility>
#include <functional>

#include "widget.hxx"
#include "container.hxx"

namespace eggui
{
// TODO Add support for providing generated input testing purposes.
class Window
{
public:
	Window(std::unique_ptr<Container> container)
		: root_container(std::move(container))
	{
	}

	/// @brief Set window title
	/// @param title_str Title string, null terminated.
	void set_title(const char *title_str) { title = title_str; }

	/// @brief Action to be performed when closing
	/// @param callback Callback, window is closed if it returns true.
	void set_close_action(std::function<bool()> callback)
	{
		close_action = callback;
	}

	void set_debug(bool enable) { debug_borders_enabled = enable; }

	/// @brief Create the window and start handling input events.
	/// @param width_hint Desired window width (negative for auto).
	/// @param height_hint Desired window height (negative for auto).
	void main_loop(int width_hint = -1, int height_hint = -1);

private:
	/// @brief Update all the widgets by sending events to them.
	void update();
	/// @brief Draw the window.
	void draw();
	/// @brief Layout the widgets.
	/// @param size Size of the window for layout.
	void layout(Point size);
	/// @brief Set min and max window size as per root_container size.
	void set_resize_limits();

	const char *title = "EGGUI Window";
	std::unique_ptr<Container> root_container;
	std::function<bool()> close_action = []() { return true; };

	bool debug_borders_enabled = false;
	// Number of times widgets should be drawn after a change.
	int draw_cnt = 1;

	// Widget over which mouse button has been pressed but not released yet.
	Widget *mouse_down_over = nullptr;
	// Widget over which the cursor is placed, if any mouse button is pressed
	// then it will be same as `mouse_down_over`.
	Widget *hovering_over = nullptr;
	// Widget over which keyboard focuses, all keypressed will to sent to it.
	Widget *focused_on = nullptr;
};
} // namespace eggui

#endif
