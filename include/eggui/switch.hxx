#ifndef SWITCH_HXX_INCLUDED
#define SWITCH_HXX_INCLUDED

#include <functional>

#include "widget.hxx"

namespace eggui
{
class Switch : public Interactive
{
public:
	using ToggleCallback = std::function<void(Window &, Switch &, bool)>;

	Switch(int w, int h, bool init_state = false)
		: Interactive(w, h)
		, state(init_state)
	{
	}

	/// @brief Set state of the switch
	/// @param new_state New state
	/// @param window Window, needed for animation and callback to on_toggle.
	void set_state(bool new_state, Window &window);
	/// @brief Get current ON/OFF state of the switch
	/// @return Switch state.
	bool get_state() const { return state; }

	void set_on_toggle(ToggleCallback callback) { on_toggle = callback; }

protected:
	Widget *notify(Event ev) override;
	void draw() override;

private:
	ToggleCallback on_toggle = [](auto &, auto &, bool) {};
	// Fraction circular slider has moved. Range: 0 to 1.
	float slider_pos = 0;
	bool state = false;
};
} // namespace eggui

#endif
