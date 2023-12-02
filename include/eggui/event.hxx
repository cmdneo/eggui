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
	// Keyboard events
	KeyDown,
	KeyReleased,
	KeyPressed,
};

struct Event {
	Event(EventType ev_type)
		: type(ev_type)
	{
	}

	EventType type;

	// Event data, C++ supports anonymous unions.
	union {
		Point delta;
		Point position;
		Point scroll;
		int key;
	};
};
} // namespace eggui

#endif