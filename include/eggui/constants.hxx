#ifndef CONSTANTS_HXX_INCLUDED
#define CONSTANTS_HXX_INCLUDED

namespace eggui
{
// Types for widget layout inside a container
//---------------------------------------------------------
enum class Alignment {
	Start,
	Center,
	End,
};

enum class Fill {
	None,
	Row,
	Column,
	RowNColumn,
};

// Types for configuring various widget properties
//-------------------------------------------------------------------
enum class Orientation {
	Horizontal = 0,
	Vertical = 1,
};

enum class Direction : unsigned {
	Top = 1 << 0,
	Bottom = 1 << 1,
	Left = 1 << 2,
	Right = 1 << 3,
	TopLeft = Top | Left,
	TopRight = Top | Right,
	BottomLeft = Bottom | Left,
	BottomRight = Bottom | Right,
};

inline bool has_component(Direction dir, Direction comp)
{
	return (static_cast<unsigned>(dir) & static_cast<unsigned>(comp)) != 0;
}

// Constants and properties
//-------------------------------------------------------------------
/// Ticks per second for update interval(if any) of widgets.
constexpr int TICKS_PER_SECOND = 30;
constexpr double UPDATE_DELTA_TIME = 1.0 / TICKS_PER_SECOND;

// TODO Use a better method for arbitrarily growable widgets.
/// Represents size for an arbtriararily growable widget.
// Using a large enough value but not the maximum ensures no overflow when
// doing arithmetic with it, and a large enough size for all screens.
constexpr int UNLIMITED_MAX_SIZE = 1e6;
} // namespace eggui

#endif