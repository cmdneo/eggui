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

	void set_size(Point size) override;

protected:
	auto notify(Event ev) -> Widget * override;
	void draw() override;
	void draw_debug() override;

private:
	EditableTextBox text;
};
} // namespace eggui

#endif