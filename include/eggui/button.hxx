#ifndef BUTTON_HXX_INCLUDED
#define BUTTON_HXX_INCLUDED

#include <functional>

#include "widget.hxx"
#include "event.hxx"

namespace eggui
{
class Button : public Interactive
{
public:
	Button(int w, int h, const char *txt)
		: Interactive(w, h)
		, label(txt)
	{
	}

	void set_text(const char *txt) { label = txt; }

	void set_on_click(std::function<void(Button &)> callback)
	{
		on_click = callback;
	}

	Widget *notify(Event ev) override;
	void draw() override;

private:
	std::function<void(Button &)> on_click = [](auto &) {};
	const char *label = "";
};
} // namespace eggui

#endif