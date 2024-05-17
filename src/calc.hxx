/// Includes functions for doing basic layout calculations. Internal use only.

#ifndef CALC_HXX_INCLUDED
#define CALC_HXX_INCLUDED

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
Point calc_align_offset(
	Point child_size, Point cont_size, Alignment halign, Alignment valign
);

/// @brief Calculates stretched size of the box within another box.
/// @param min_size Minimum size of the inner box.
/// @param max_size Maximum size of the inner box.
/// @param avail_size Size of the outer box.
/// @param fill Fill mode for stretching.
/// @return Calculated size.
/// For each dimension: if `size_avail < min_size` then returns `min_size`.
Point calc_stretched_size(
	Point min_size, Point max_size, Point avail_size, Fill fill
);

/// @brief Calculates offsets of boxes for layout and puts them is `result`.
/// @param sizes Length of each box.
/// @param gap Gap between boxes along the length.
/// @param result Result parameter: offsets of boxes from 0.
/// @return Total length of the container containing the boxes.
int calc_box_offsets(
	const std::vector<int> &sizes, int gap, std::vector<int> &result
);

/// @brief Calculate how much space should be used by each box.
/// @param min_sizes Minimum size of each box.
/// @param max_sizes Maximum size of each box.
/// @param avail_size Total available size.
/// @param result Result argument, resultant size of each.
void calc_expanded_size(
	const std::vector<int> &min_sizes, const std::vector<int> &max_sizes,
	int avail_size, std::vector<int> &result
);

/// @brief Calculate length of the container taking in accout gaps in between.
/// @param lens Lengths of the boxes.
/// @param gap Gap between each box.
/// @return Total length.
int calc_length_with_gaps(const std::vector<int> &lenghts, int gap);
} // namespace eggui

#endif
