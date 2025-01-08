#ifndef DRAGGABLE_HXX_INCLUDED
#define DRAGGABLE_HXX_INCLUDED

#include <functional>

#include "widget.hxx"

namespace eggui
{
class Draggable : public Interactive
{
public:
	using DragCallback = std::function<void(Window &, Draggable &, Point)>;

	Draggable(int w, int h)
		: Interactive(w, h)
	{
	}

	void set_on_drag(DragCallback callback) { on_drag = callback; }

protected:
	auto notify(Event ev) -> Widget * override;
	void draw() override;

private:
	DragCallback on_drag = [](auto &, auto &, auto) {};
};
} // namespace eggui

#endif