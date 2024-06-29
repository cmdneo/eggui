#ifndef LABEL_HXX_INCLUDED
#define LABEL_HXX_INCLUDED

#include <string>

#include "widget.hxx"

namespace eggui
{
class Label final : public Widget
{
public:
	Label(
		int w, int h, std::string text_, RGBA color_,
		FontSize font_size_ = FontSize::Medium
	)
		: Widget(w, h)
		, text(text_)
		, color(color_)
		, font_size(font_size_)
	{
	}

	/// @brief Set alignment.
	/// @param halign Horizontal alignment.
	/// @param valign Vertical alignment.
	void set_text_align(Alignment halign, Alignment valign)
	{
		h_align = halign;
		v_align = valign;
	}

	void set_color(RGBA color_) { color = color_; }
	void set_text(std::string txt);

protected:
	void draw() override;

private:
	std::string text = "";
	RGBA color;
	FontSize font_size;
	Alignment h_align = Alignment::Start;
	Alignment v_align = Alignment::Start;
};
} // namespace eggui

#endif