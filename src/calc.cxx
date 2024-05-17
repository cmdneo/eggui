#include <cassert>
#include <algorithm>
#include <vector>

#include "calc.hxx"
#include "widget.hxx"

namespace eggui
{
Point calc_align_offset(
	Point child_size, Point cont_size, Alignment halign, Alignment valign
)
{
	auto align_pos = [](int outer, int inner, Alignment align) -> int {
		switch (align) {
		case Alignment::Start:
			return 0;
		case Alignment::Center:
			return (outer - inner) / 2;
		case Alignment::End:
			return outer - inner;
		}
		assert(!"unreachable");
		return 0;
	};

	return Point(
		align_pos(cont_size.x, child_size.x, halign),
		align_pos(cont_size.y, child_size.y, valign)
	);
}

Point calc_stretched_size(
	Point min_size, Point max_size, Point avail_size, Fill fill
)
{
	max_size = min_components(avail_size, max_size);

	switch (fill) {
	case Fill::Column:
		return Point(max_size.x, min_size.y);
	case Fill::Row:
		return Point(min_size.x, max_size.y);
	case Fill::RowNColumn:
		return max_size;
	case Fill::None:
		return min_size;
	}

	assert(!"unreachable");
	return Point(0, 0);
}

int calc_box_offsets(
	const std::vector<int> &sizes, int gap, std::vector<int> &result
)
{
	result.resize(sizes.size());

	int last_at = 0;
	for (unsigned i = 0; i < sizes.size(); ++i) {
		result[i] = last_at;
		last_at += sizes[i] + gap;
	}
	last_at -= gap;

	return last_at;
}

void calc_expanded_size(
	const std::vector<int> &min_sizes, const std::vector<int> &max_sizes,
	int avail_size, std::vector<int> &result
)
{
	assert(min_sizes.size() == max_sizes.size());
	result.resize(min_sizes.size());

	int min_size = 0;
	for (auto v : min_sizes)
		min_size += v;

	int max_size = 0;
	for (auto v : max_sizes)
		max_size += v;

	int usable_size = std::min(max_size, avail_size);

	// If none of the boxes can be expanded then just set it to zero.
	double increment = 0;
	if (int diff = max_size - min_size; diff > 0)
		increment = 1.0 * (usable_size - min_size) / diff;

	int unused_size = usable_size;
	for (unsigned i = 0; i < min_sizes.size(); ++i) {
		int extra = max_sizes[i] - min_sizes[i];
		double actual = min_sizes[i] + increment * extra;
		result[i] = min_sizes[i] + increment * extra;
		unused_size -= result[i];
	}

	// Since conversion to int truncates the fractional part we add the
	// remaining unused size so that we use all the usable space.
	// Beacause fractional < 1, therefore: unused_size <= number_of_elements.
	assert(unused_size <= static_cast<int>(result.size()));
	for (int i = 0; i < unused_size; ++i)
		result[i]++;
}

int calc_length_with_gaps(const std::vector<int> &lengths, int gap)
{
	int sum = 0;
	for (auto v : lengths)
		sum += v;

	return sum + gap * (lengths.size() - 1);
};
} // namespace eggui
