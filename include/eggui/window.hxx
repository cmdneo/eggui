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

	void main_loop(int width, int height);

private:
	/// @brief Update all the widgets by sending events to them.
	void update();
	/// @brief Draw the window
	void draw();

	const char *title = "EGGUI - :)";
	std::unique_ptr<Container> root_container;
	std::function<bool()> close_action = []() { return true; };

	bool debug_borders_enabled = false;
	bool needs_redraw = true;

	// If mouse is down over a widget then no event is sent to others.
	Widget *mouse_down_over = nullptr;
	Widget *hovering_over = nullptr;
	Widget *focused_on = nullptr;
};
} // namespace eggui

#endif