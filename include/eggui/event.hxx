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
	Event(EventType ev_type)
		: type(ev_type)
	{
	}

	EventType type;

	// --- Event data ---
	// Absolute cursor position
	Point cursor;
	// We use a union because events using it are exclusive.
	union {
		Point shared_pt_;
		Point delta;
		Point scroll;
		int key;
	};
};
} // namespace eggui

#endif