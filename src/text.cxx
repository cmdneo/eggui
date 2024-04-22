#include <cassert>
#include <algorithm>
#include <string>
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
	, text(std::move(txt))
	, font_size(font_size_)
{
	calc_cursor_offset();
}

void EditableTextBox::set_font_size(FontSize size)
{
	font_size = size;
	calc_cursor_offset();
}

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

	cursor_xpos -= get_char_width(text[cursor_at - 1], font_size);
	text.erase(text.begin() + cursor_at - 1);
	cursor_at--;
}

Widget *EditableTextBox::notify_impl(Event ev)
{
	if (ev.type != EventType::MousePressed)
		return Interactive::notify_impl(ev);

	float xpos = 0;
	for (unsigned i = 0; i < text.size(); ++i) {
		float width = get_char_width(text[i], font_size);

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

void EditableTextBox::draw_impl()
{
	constexpr int DRAW_BUF_SIZE = 255;
	char draw_buf[DRAW_BUF_SIZE + 1]{};

	draw_text(Point(0, 0), TEXT_COLOR, text.c_str(), font_size);

	if (!cursor_visible)
		return;
	// Draw the cursor
	Point size(2, font_size_to_pixels(font_size));
	draw_rect(Point(cursor_xpos, 0), size, CURSOR_COLOR);
}

void EditableTextBox::calc_cursor_offset()
{
	assert(cursor_at >= 0 && static_cast<unsigned>(cursor_at) <= text.size());

	float pos = 0;
	for (int i = 0; i < cursor_at; ++i)
		pos += get_char_width(text[i], font_size);

	cursor_xpos = pos;
}
