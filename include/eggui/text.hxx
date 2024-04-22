#ifndef TEXT_HXX_INCLUDED
#define TEXT_HXX_INCLUDED

#include <string>
#include <vector>
#include <utility>
#include <optional>

#include "widget.hxx"
#include "graphics.hxx"

namespace eggui
{
class EditableTextBox : public Interactive
{
public:
	EditableTextBox(
		int w, int h, std::string txt, FontSize font_size_ = FontSize::Medium
	);

	void set_font_size(FontSize size);
	void set_cursor(bool visibility) { cursor_visible = visibility; }

	/// @brief Move cursor by a specified position left or right.
	///        Clamps the cursor position if delta is too high.
	/// @param delta Amount to move. Positive: right, Negative: left.
	/// @return The actual amount(delta) cursor was moved.
	int move_cursor(int delta);
	void insert_before_cursor(char c);
	void delete_before_cursor();

protected:
	Widget *notify_impl(Event ev) override;
	void draw_impl() override;

private:
	void calc_cursor_offset();

	// Text value
	std::string text;
	// Region of the text which is selected(if any). Range is [low, high).
	std::optional<std::pair<int, int>> selected_range;
	// Calculated cursor offset from the begining of text in pixels.
	float cursor_xpos = 0;
	// Cursor position within the text.
	int cursor_at = 0;
	bool cursor_visible = true;
	FontSize font_size = FontSize::Medium;
};
} // namespace eggui

#endif