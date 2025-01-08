#ifndef TOAST_HXX_INCLUDED
#define TOAST_HXX_INCLUDED

#include <utility>
#include <string>

#include "widget.hxx"
#include "label.hxx"

namespace eggui
{
class Toast : public Widget
{
public:
	Toast(
		int w, int h, float show_time_, std::string msg,
		Direction corner_ = Direction::TopLeft
	);

	/// @brief Show the toast on the window.
	/// @param window Window.
	void show_toast(Window &window);

	void set_size(Point size) override;
	void set_show_timer_bar(bool visibility) { show_timer_bar = visibility; }
	void set_timer_bar_spent(float spent);

protected:
	void draw() override;

private:
	Label message;
	Direction corner = Direction::Top;
	bool show_timer_bar = true;
	// From 0(full) to 1(time over).
	float timer_bar_spent = 0;
	float show_time = 0;
};
} // namespace eggui

#endif