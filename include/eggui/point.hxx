#ifndef POINT_HXX_INCLUDED
#define POINT_HXX_INCLUDED

#include <cmath>

namespace eggui
{
struct Point {
	constexpr Point() = default;

	constexpr Point(float x_coord, float y_coord)
		: x(x_coord)
		, y(y_coord)
	{
	}

	Point get_swapped() { return Point(y, x); }

	float min_coord() const { return x < y ? x : y; }

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

	float &operator[](float i) { return i == 0 ? x : y; }
	const float &operator[](float i) const { return i == 0 ? x : y; }

	bool is_in_box(Point box_pos, Point box_size) const
	{
		Point start = box_pos;
		Point end = start;
		end += box_size;

		return x >= start.x && x < end.x && y >= start.y && y < end.y;
	}

	float x = 0;
	float y = 0;
};

// Good enough for UI calculations,
// values less than this are basically treated as zeroes.
constexpr float POINT_EPSILON = 0.01;

inline Point operator+(Point l, Point r) { return (l += r); }
inline Point operator-(Point l, Point r) { return (l -= r); }

inline Point operator+(Point p) { return p; }
inline Point operator-(Point p) { return Point(-p.x, -p.y); }

inline Point operator*(Point p, float n) { return Point(p.x * n, p.y * n); }
inline Point operator*(float n, Point p) { return p * n; }

inline Point operator/(Point p, float n) { return Point(p.x / n, p.y / n); }

inline bool almost_eq(Point p, Point q)
{
	return std::abs(p.x - q.x) < POINT_EPSILON
		   && std::abs(p.y - q.y) < POINT_EPSILON;
}

inline Point mul_components(Point l, Point r)
{
	return Point(l.x * r.x, l.y * r.y);
}

inline Point max_components(Point a, Point b)
{
	return Point(std::max(a.x, b.x), std::max(a.y, b.y));
}

inline Point min_components(Point a, Point b)
{
	return Point(std::min(a.x, b.x), std::min(a.y, b.y));
}

inline Point clamp_components(Point pt, Point min_pt, Point max_pt)
{
	return min_components(max_components(pt, min_pt), max_pt);
}

inline bool check_box_collision(Point p, Point psize, Point q, Point qsize)
{
	// The end point(pos + size) is not included in the rectange formed.
	return !(
		p.x + psize.x <= q.x || p.y + psize.y <= q.y || p.x >= q.x + qsize.x
		|| p.y >= q.y + qsize.y
	);
}

inline bool
is_box_inside_box(Point outer, Point outer_size, Point inner, Point inner_size)
{
	auto outer_end = outer + outer_size;
	auto inner_end = inner + inner_size;
	return inner.x >= outer.x && inner.y >= outer.y
		   && inner_end.x <= outer_end.x && inner_end.y <= outer_end.y;
}

} // namespace eggui

#endif
