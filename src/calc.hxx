#ifndef CALC_HXX_INCLUDED
#define CALC_HXX_INCLUDED

// Includes functions for doing basic layout calculations. Internal use only.

#include <cassert>
#include <algorithm>
#include <vector>

#include "widget.hxx"

namespace eggui
{
/// @brief Calculates the relative-position of child inside contatiner as per
/// horinzontal and vertical alingments.
/// @param child_size Size of the inner box.
/// @param cont_size Size of the container.
/// @param halign Horizontal alignment.
/// @param valign Vertical alignment.
/// @return Position of the child within container.
inline Point calc_align_offset(
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

/// @brief Calculates stretched size of the box within another box.
/// @param min_size Minimum size of the inner box.
/// @param max_size Maximum size of the inner box.
/// @param avail_size Size of the outer box.
/// @param fill Fill mode for stretching.
/// @return Calculated size.
/// For each dimension: if `size_avail < min_size` then returns `min_size`.
inline Point
calc_stretched_size(Point min_size, Point max_size, Point avail_size, Fill fill)
{
	avail_size = clamp_components(avail_size, min_size, max_size);

	switch (fill) {
	case Fill::StretchColumn:
		return Point(avail_size.x, min_size.y);
	case Fill::StretchRow:
		return Point(min_size.x, avail_size.y);
	case Fill::Stretch:
		return avail_size;
	case Fill::None:
		return min_size;
	}

	assert(!"unreachable");
	return Point(0, 0);
}

/// @brief Calculates offsets of boxes for layout and puts them is `result`.
/// @param sizes Length of each box.
/// @param gap Gap between boxes along the length.
/// @param result Result parameter: offsets of boxes from 0.
/// @return Total length of the container containing the boxes.
inline float calc_box_offsets(
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

/// @brief Calculate how much `flex_size` should expand to fill the `size_avail`
/// @param box_size Size of the box.
/// @param flex_size Size wrt which expansion should be calculated.
/// @param avail_size Size available to the box.
inline void calc_expanded_size(
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
	if (auto diff = max_size - min_size; diff > 0.1)
		inc_frac = (avail_size - min_size) / diff;

	for (unsigned i = 0; i < min_sizes.size(); ++i) {
		double extra = max_sizes[i] - min_sizes[i];
		result[i] = min_sizes[i] + inc_frac * extra;
	}
}
/// @brief Calculate length of the container taking in accout gaps in between.
/// @param lens Lengths of the boxes.
/// @param gap Gap between each box.
/// @return Total length.
inline float calc_gapped_length(const std::vector<float> &lens, int gap)
{
	float sum = 0;
	for (auto v : lens)
		sum += v;

	return sum + gap * (lens.size() - 1);
};
} // namespace eggui

#endif
