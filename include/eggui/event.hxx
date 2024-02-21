#ifndef EVENT_HXX_INCLUDED
#define EVENT_HXX_INCLUDED

#include "point.hxx"

namespace eggui
{
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
	KeyDown,
	KeyReleased,
	KeyPressed,
	// Events for querying
	IsInteractive,
};

struct Event {
	Event(EventType ev_type, Point cursor_ = Point(0, 0))
		: type(ev_type)
		, cursor(cursor_)
	{
	}

	EventType type;

	// --- Event data ---
	// Absolute cursor position
	Point cursor;
	// We use a union because events using it are exclusive.
	union {
		Point delta;
		Point scroll;
		int key;
	};
};
} // namespace eggui

#endif