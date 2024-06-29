#ifndef BUTTON_HXX_INCLUDED
#define BUTTON_HXX_INCLUDED

#include <string>
#include <functional>
#include <utility>

#include "widget.hxx"
#include "label.hxx"

namespace eggui
{
class Button : public Interactive
{
public:
	Button(int w, int h, std::string txt);

	void set_label(std::string txt) { label.set_text(std::move(txt)); }

	void set_on_click(std::function<void(Window &, Button &)> callback)
	{
		on_click = callback;
	}

	void set_size(Point new_size) override;

protected:
	Widget *notify(Event ev) override;
	void draw() override;

private:
	std::function<void(Window &, Button &)> on_click = [](auto &, auto &) {};
	Label label;
};
} // namespace eggui

#endif