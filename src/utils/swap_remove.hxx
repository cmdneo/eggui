#ifndef UTILS_SWAP_REMOVE_HXX
#define UTILS_SWAP_REMOVE_HXX

#include <functional>
#include <utility>

namespace eggui
{
template <typename T>
void swap_remove(T &container, typename T::size_type index)
{
	std::swap(container[index], container.back());
	container.pop_back();
}

template <typename T>
void swap_remove_if(
	T &container, std::function<bool(typename T::value_type &)> unary_pred
)
{
	typename T::size_type i = 0;

	while (i != container.size()) {
		if (unary_pred(container[i]))
			swap_remove(container, i);
		else
			i++;
	}
}
} // namespace eggui

#endif
