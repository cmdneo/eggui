#ifndef POINT_HXX_INCLUDED
#define POINT_HXX_INCLUDED

namespace eggui
{
struct Point {
	constexpr Point() = default;

	constexpr Point(int x_coord, int y_coord)
		: x(x_coord)
		, y(y_coord)
	{
	}

	Point &operator+=(Point other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	Point &operator-=(Point other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	int x = 0;
	int y = 0;
};

inline Point operator+(Point l, Point r) { return (l += r); }
inline Point operator-(Point l, Point r) { return (l -= r); }
inline bool operator==(Point l, Point r) { return l.x == r.x && l.y == r.y; }

inline Point max_components(Point a, Point b)
{
	return Point(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
}
} // namespace eggui

#endif
