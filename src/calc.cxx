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
	avail_size = clamp_components(avail_size, min_size, max_size);

	switch (fill) {
	case Fill::Column:
		return Point(avail_size.x, min_size.y);
	case Fill::Row:
		return Point(min_size.x, avail_size.y);
	case Fill::RowNColumn:
		return avail_size;
	case Fill::None:
		return min_size;
	}

	assert(!"unreachable");
	return Point(0, 0);
}

float calc_box_offsets(
	const std::vector<float> &sizes, float gap, std::vector<float> &result
)
{
	result.resize(sizes.size());

	double last_at = 0.;
	for (unsigned i = 0; i < sizes.size(); ++i) {
		result[i] = last_at;
		last_at += sizes[i] + gap;
	}
	last_at -= gap;

	return last_at;
}

void calc_expanded_size(
	const std::vector<float> &min_sizes, const std::vector<float> &max_sizes,
	float avail_size, std::vector<float> &result
)
{
	assert(min_sizes.size() == max_sizes.size());
	result.resize(min_sizes.size());

	double min_size = 0;
	for (auto v : min_sizes)
		min_size += v;

	double max_size = 0;
	for (auto v : max_sizes)
		max_size += v;

	avail_size = std::min(float(max_size), avail_size);

	// If none of the boxes can be expanded then just set it to zero.
	double inc_frac = 0;
	if (auto diff = max_size - min_size; diff > POINT_EPSILON)
		inc_frac = (avail_size - min_size) / diff;

	for (unsigned i = 0; i < min_sizes.size(); ++i) {
		double extra = max_sizes[i] - min_sizes[i];
		result[i] = min_sizes[i] + inc_frac * extra;
	}
}

float calc_length_with_gaps(const std::vector<float> &lens, int gap)
{
	float sum = 0;
	for (auto v : lens)
		sum += v;

	return sum + gap * (lens.size() - 1);
};

} // namespace eggui
