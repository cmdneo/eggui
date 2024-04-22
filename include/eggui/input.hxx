#ifndef INPUT_HXX_INCLUDED
#define INPUT_HXX_INCLUDED

#include "widget.hxx"
#include "text.hxx"

namespace eggui
{
class TextInput final : public Interactive
{
public:
	TextInput(int w, int h);

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;

private:
	EditableTextBox text;
	bool cursor_vis = true;
};
} // namespace eggui

#endif