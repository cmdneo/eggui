#ifndef POINT_HXX_INCLUDED
#define POINT_HXX_INCLUDED

#include <algorithm>

namespace eggui
{
struct Point {
	constexpr Point() = default;

	constexpr Point(int x_coord, int y_coord)
		: x(x_coord)
		, y(y_coord)
	{
	}

	auto get_swapped() -> Point { return {y, x}; }

	[[nodiscard]] auto min_coord() const -> int { return x < y ? x : y; }

	auto operator+=(Point other) -> Point &
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	auto operator-=(Point other) -> Point &
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	auto operator[](int i) -> int & { return i == 0 ? x : y; }
	auto operator[](int i) const -> const int & { return i == 0 ? x : y; }

	[[nodiscard]] auto is_in_box(Point box_pos, Point box_size) const -> bool
	{
		Point start = box_pos;
		Point end = start;
		end += box_size;

		return x >= start.x && x < end.x && y >= start.y && y < end.y;
	}

	int x = 0;
	int y = 0;
};

inline auto operator+(Point l, Point r) -> Point { return (l += r); }
inline auto operator-(Point l, Point r) -> Point { return (l -= r); }

inline auto operator+(Point p) -> Point { return p; }
inline auto operator-(Point p) -> Point { return {-p.x, -p.y}; }

inline auto operator*(Point p, double n) -> Point
{
	return {static_cast<int>(p.x * n), static_cast<int>(p.y * n)};
}
inline auto operator*(double n, Point p) -> Point { return p * n; }

inline auto operator/(Point p, double n) -> Point
{
	return {static_cast<int>(p.x / n), static_cast<int>(p.y / n)};
}

inline auto operator==(Point p, Point q) -> bool
{
	return p.x == q.x && p.y == q.y;
}

inline auto mul_components(Point l, Point r) -> Point
{
	return {l.x * r.x, l.y * r.y};
}

inline auto max_components(Point a, Point b) -> Point
{
	return {std::max(a.x, b.x), std::max(a.y, b.y)};
}

inline auto min_components(Point a, Point b) -> Point
{
	return {std::min(a.x, b.x), std::min(a.y, b.y)};
}

inline auto clamp_components(Point pt, Point min_pt, Point max_pt) -> Point
{
	return min_components(max_components(pt, min_pt), max_pt);
}

inline auto check_box_collision(Point p, Point psize, Point q, Point qsize)
	-> bool
{
	// The end point(pos + size) is not included in the rectange formed.
	return !(
		p.x + psize.x <= q.x || p.y + psize.y <= q.y || p.x >= q.x + qsize.x
		|| p.y >= q.y + qsize.y
	);
}

inline auto
is_box_inside_box(Point outer, Point outer_size, Point inner, Point inner_size)
	-> bool
{
	auto outer_end = outer + outer_size;
	auto inner_end = inner + inner_size;
	return inner.x >= outer.x && inner.y >= outer.y
		   && inner_end.x <= outer_end.x && inner_end.y <= outer_end.y;
}
} // namespace eggui

#endif
