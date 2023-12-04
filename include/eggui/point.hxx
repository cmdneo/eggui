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

	bool is_in_box(Point box_pos, Point box_size)
	{
		Point start = box_pos;
		Point end = start;
		end += box_size;

		return x >= start.x && x < end.x && y >= start.y && y < end.y;
	}

	int x = 0;
	int y = 0;
};

inline Point operator+(Point l, Point r) { return (l += r); }
inline Point operator-(Point l, Point r) { return (l -= r); }
inline Point operator-(Point l) { return Point(-l.x, -l.y); }
inline bool operator==(Point l, Point r) { return l.x == r.x && l.y == r.y; }

inline Point max_components(Point a, Point b)
{
	return Point(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
}

inline bool check_box_collision(Point p, Point psize, Point q, Point qsize)
{
	// The end point(pos + size) is not included in the rectange formed.
	return !(
		p.x + psize.x <= q.x || p.y + psize.y <= q.y || p.x >= q.x + qsize.x
		|| p.y >= q.y + qsize.y
	);
}
} // namespace eggui

#endif
