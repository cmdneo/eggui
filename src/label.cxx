#include <string>
#include <utility>

#include "widget.hxx"
#include "label.hxx"
#include "graphics.hxx"
#include "calc.hxx"

using namespace eggui;

void Label::set_text(std::string txt) { text = std::move(txt); }

void Label::draw_impl()
{
	auto cstr = text.c_str();

	auto size = tell_text_size(cstr, font_size);
	auto pos = calc_align_offset(size, get_size(), h_align, v_align);
	draw_text(pos, color, cstr, font_size);
}