#include <cassert>
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "widget.hxx"
#include "text.hxx"
#include "theme.hxx"
#include "graphics.hxx"

using namespace eggui;

inline int get_sign(int s) { return s != 0 ? s / std::abs(s) : 0; }

static float get_char_width(char c, FontSize size)
{
	const char s[] = {c, 0};
	return tell_text_size(s, size).x;
}

EditableTextBox::EditableTextBox(
	int w, int h, std::string txt, FontSize font_size_
)
	: Interactive(w, h)
	, font_size(font_size_)
{
	set_text(std::move(txt));
}

void EditableTextBox::set_font_size(FontSize size)
{
	font_size = size;
	calc_cursor_offset();
}

void EditableTextBox::set_cursor_opacity(float opacity)
{
	cursor_opacity = std::clamp(255. * opacity, 0., 255.);
}

void EditableTextBox::set_text(std::string txt)
{
	text = std::move(txt);
	cursor_at = 0;
	cursor_xpos = 0;
}

std::string_view EditableTextBox::get_text() const { return text; }

int EditableTextBox::move_cursor(int delta)
{
	int new_pos =
		std::clamp(cursor_at + delta, 0, static_cast<int>(text.size()));
	delta = new_pos - cursor_at;

	// Update cursor x-position as we move the cursor over characters.
	for (int i = 0; i < std::abs(delta); ++i) {
		if (delta > 0) {
			cursor_xpos += get_char_width(text[cursor_at + i], font_size);
		} else {
			cursor_xpos -= get_char_width(text[cursor_at - i - 1], font_size);
		}
	}

	cursor_at = new_pos;
	return delta;
}

void EditableTextBox::insert_before_cursor(char c)
{
	cursor_xpos += get_char_width(c, font_size);
	text.insert(text.begin() + cursor_at, c);

	cursor_at++;
}

void EditableTextBox::delete_before_cursor()
{
	if (cursor_at == 0)
		return;

	char c = text[cursor_at - 1];
	cursor_xpos -= get_char_width(c, font_size);

	text.erase(text.begin() + cursor_at - 1);
	cursor_at--;
}

void EditableTextBox::delete_after_cursor()
{
	if (cursor_at == static_cast<int>(text.size()))
		return;

	text.erase(text.begin() + cursor_at);
}

Widget *EditableTextBox::notify(Event ev)
{
	if (ev.type != EventType::MousePressed)
		return Interactive::notify(ev);

	float xpos = 0;
	for (unsigned i = 0; i < text.size(); ++i) {
		float width = get_char_width(text[i], font_size);

		// TODO Add support correct cursor position on scrolled text.
		// Move the cursor to the left of char if clicked in left-half, and
		// move the cursor to the right of the char if pressed in right-half.
		if (xpos > ev.cursor.x) {
			// Nothing...
		} else if (ev.cursor.x < xpos + width / 2) {
			move_cursor(i - cursor_at);
			return this;
		} else if (ev.cursor.x < xpos + width) {
			move_cursor(i + 1 - cursor_at);
			return this;
		}

		xpos += width;
	}

	move_cursor(text.size() - cursor_at);
	return this;
}

void EditableTextBox::draw()
{
	// constexpr int DRAW_BUF_SIZE = 255;
	// char draw_buf[DRAW_BUF_SIZE + 1]{};

	// Scroll the text to the cursor position on overflow.
	float x_offset = std::min(0.f, get_size().x - cursor_xpos - CURSOR_WIDTH);
	draw_text(Point(x_offset, 0), TEXT_COLOR, text.c_str(), font_size);

	// Draw the cursor
	auto ccol = CURSOR_COLOR;
	ccol.a = cursor_opacity;
	Point size(CURSOR_WIDTH, font_size_to_pixels(font_size));
	draw_rect(Point(x_offset + cursor_xpos, 0), size, ccol);
}

void EditableTextBox::calc_cursor_offset()
{
	assert(cursor_at >= 0 && static_cast<unsigned>(cursor_at) <= text.size());

	float pos = 0;
	for (int i = 0; i < cursor_at; ++i)
		pos += get_char_width(text[i], font_size);

	cursor_xpos = pos;
}
