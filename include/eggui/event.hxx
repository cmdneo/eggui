#ifndef EVENT_HXX_INCLUDED
#define EVENT_HXX_INCLUDED

#include "point.hxx"

namespace eggui
{
// Forward delcaration
class Window;

enum class EventType {
	// Mouse events
	Scroll,
	MouseIn,
	MouseOut,
	MousePressed,
	MouseReleased,
	MouseClick,
	MouseDrag,
	MouseMotion,
	// Keyboard events
	KeyPressed,
	CharEntered,
	// Events for querying
	IsInteractive,
};

struct Event {
	Event(Window &win, EventType ev_type, Point cursor_)
		: window(win)
		, type(ev_type)
		, cursor(cursor_)
	{
	}

	Event(Window &win, EventType ev_type, int key_or_char)
		: window(win)
		, type(ev_type)
		, keycode(key_or_char)
	{
	}

	Window &window;
	EventType type;

	Point cursor;
	// We use a union because events using it are exclusive.
	union {
		Point delta;
		Point scroll;
		// Keycode is dependent on the library used(here raylib).
		int keycode;
		int char_val;
	};
};
} // namespace eggui

#endif