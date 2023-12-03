#ifndef BUTTON_HXX_INCLUDED
#define BUTTON_HXX_INCLUDED

#include <functional>

#include "widget.hxx"
#include "event.hxx"

namespace eggui
{
class Button : public InteractiveWidget
{
public:
	Button(int w, int h, const char *txt)
		: InteractiveWidget(w, h)
		, label(txt)
	{
	}

	void set_text(const char *txt) { label = txt; }

	void set_click_action(std::function<void(Button &)> callback)
	{
		click_action = callback;
	}

	void draw() override;
	bool notify(Event ev) override;

private:
	std::function<void(Button &)> click_action = [](auto &) {};
	const char *label = "";

	bool is_hovering = false;
	bool is_pressed = false;
};
} // namespace eggui

#endif